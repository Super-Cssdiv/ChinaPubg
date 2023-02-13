#include <unistd.h>
#include <sys/mman.h>
#include "hide_utils.h"

bool HideTools::HideSoinfo(const std::string &name) {
    bool isHide = false;
    for (soinfo *si = solist_get_head(); si != nullptr; si = si->next) {
        const char *realpath = get_realpath(si);
        if (strstr(realpath, name.c_str()) != nullptr) {
            solist_remove_soinfo(si);
            isHide = true;
        }
    }
    return isHide;
}

std::vector<const char *> HideTools::GetSoinfoList() {
    std::vector <const char *> SoInfoList;
    for (soinfo *si = solist_get_head(); si != nullptr; si = si->next) {
        const char *realpath = get_realpath(si);
        if (realpath == nullptr)
            continue;
        SoInfoList.push_back(realpath);
    }
    return SoInfoList;
}

const char *HideTools::get_realpath(soinfo *so) {
    static const char *(*get_realpath_sym)(soinfo *);
    if (get_realpath_sym == nullptr) {
        *(uint64_t *) &get_realpath_sym = SandHook::ElfImg("/linker").getSymbAddress("__dl__ZNK6soinfo12get_realpathEv");
    }
    if (get_realpath_sym == nullptr) {
        return nullptr;
    }
    return get_realpath_sym(so);
}

soinfo *HideTools::solist_get_head() {
    static soinfo *(*solist_get_head_sym)();
    if (solist_get_head_sym == nullptr) {
        *(uint64_t *) &solist_get_head_sym = SandHook::ElfImg("/linker").getSymbAddress("__dl__Z15solist_get_headv");
    }
    if (solist_get_head_sym == nullptr) {
        return nullptr;
    }
    return solist_get_head_sym();
}

bool HideTools::solist_remove_soinfo(soinfo *si) {
    soinfo* solist = solist_get_head();
    soinfo *sonext = solist_get_next();
    if (sonext == nullptr || solist == nullptr) {
        return false;
    }
    soinfo *prev = nullptr, *trav;
    for (trav = solist; trav != nullptr; trav = trav->next) {
        if (trav == si)
            break;
        prev = trav;
    }
    if (trav == nullptr)
        return false;
    prev->next = si->next;
    if (si == sonext)
        sonext = prev;
    return true;
}

/*bool HideTools::solist_remove_soinfo(soinfo *si) {
    static bool *(*solist_remove_soinfo_sym)(soinfo*);
    if (solist_remove_soinfo_sym == nullptr) {
        *(uint64_t *) &solist_remove_soinfo_sym = SandHook::ElfImg("/linker").getSymbAddress("__dl__Z20solist_remove_soinfoP6soinfo");
    }
    if (solist_remove_soinfo_sym == nullptr) {
        LOGW("solist_remove_soinfo == nullptr");
        return false;
    }
    return solist_remove_soinfo_sym(si);
}*/

int HideTools::getMemPermission(uint64_t address) {
    char line[PATH_MAX] = {0};
    char perms[5];
    int bol = 1;

    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == nullptr) {
        return 0;
    }

    while (fgets(line, PATH_MAX, fp) != nullptr) {
        uint64_t start, end;
        int eol = (strchr(line, '\n') != nullptr);
        if (bol) {
            if (!eol) {
                bol = 0;
            }
        } else {
            if (eol) {
                bol = 1;
            }
            continue;
        }
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) != 3) {
            continue;
        }
        if (start <= address && address < end) {
            int prot = 0;
            if (perms[0] == 'r') {
                prot |= PROT_READ;
            } else if (perms[0] != '-') {
                goto unknown_perms;
            }
            if (perms[1] == 'w') {
                prot |= PROT_WRITE;
            } else if (perms[1] != '-') {
                goto unknown_perms;
            }
            if (perms[2] == 'x') {
                prot |= PROT_EXEC;
            } else if (perms[2] != '-') {
                goto unknown_perms;
            }
            if (perms[3] != 'p') {
                goto unknown_perms;
            }
            if (perms[4] != '\0') {
                perms[4] = '\0';
                goto unknown_perms;
            }
            fclose(fp);
            return prot;
        }
    }
    unknown_perms:
    fclose(fp);
    return 0;
}

soinfo *HideTools::solist_get_next() {
    static soinfo *sonext = nullptr;
    if (sonext == nullptr) {
        sonext = SandHook::ElfImg("/linker").getSymbAddress<soinfo *>("__dl__ZL6sonext");
    }
    return sonext;
}
