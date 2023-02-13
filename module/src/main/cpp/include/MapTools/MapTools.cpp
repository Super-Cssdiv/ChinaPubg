#pragma clang diagnostic push
#pragma ide diagnostic ignored "-Wformat"
#include <MapTools.h>
#include <MemoryTools.h>
#include <EGL/egl.h>

procmaps_iterator *pmparser_parse(int pid) {
    procmaps_iterator *maps_it = (procmaps_iterator *)malloc(sizeof(procmaps_iterator));
    char maps_path[500];
    if (pid >= 0) {
        sprintf(maps_path, "/proc/%d/maps", pid);
    } else {
        sprintf(maps_path, "/proc/self/maps");
    }
    FILE *file = fopen(maps_path, "r");
    if (!file) {
        //fprintf(stderr, "pmparser : cannot open the memory maps, %s\n", strerror(errno));
        return nullptr;
    }
    int ind = 0;
    char buf[PROCMAPS_LINE_MAX_LENGTH];
    int c;
    procmaps_struct *list_maps = nullptr;
    procmaps_struct *tmp;
    procmaps_struct *current_node = list_maps;
    char addr1[20], addr2[20], perm[8], offset[20], dev[10], inode[30], pathname[PATH_MAX];
    while (!feof(file)) {
        if (fgets(buf, PROCMAPS_LINE_MAX_LENGTH, file) == nullptr) {
            //fprintf(stderr, "pmparser : fgets failed, %s\n", strerror(errno));
            //LOGE("%s, %s", "pmparser : fgets failed", buf);
//			return NULL;
        }
        //allocate a node
        tmp = (procmaps_struct *) malloc(sizeof(procmaps_struct));
        //fill the node
        _pmparser_split_line(buf, addr1, addr2, perm, offset, dev, inode, pathname);
        //printf("#%s",buf);
        //printf("%s-%s %s %s %s %s\t%s\n",addr1,addr2,perm,offset,dev,inode,pathname);
        //addr_start & addr_end
        tmp->addr_start = strtoul(addr1, nullptr,16);
        tmp->addr_end = strtoul(addr2, nullptr,16);
        //sscanf(addr1, "%lx", &tmp->addr_start);
        //sscanf(addr2, "%lx",  &tmp->addr_end);
        //size
        tmp->length = tmp->addr_end - tmp->addr_start;
        //perm
        strcpy(tmp->perm, perm);
        tmp->is_r = (perm[0] == 'r');
        tmp->is_w = (perm[1] == 'w');
        tmp->is_x = (perm[2] == 'x');
        tmp->is_p = (perm[3] == 'p');

        //offset
        tmp->offset = strtoul(offset, nullptr,16);
        //sscanf(offset, "%lx", &tmp->offset);
        //device
        strcpy(tmp->dev, dev);
        //inode
        tmp->inode = strtoul(inode,nullptr,10);
        //pathname
        strcpy(tmp->pathname, pathname);
        tmp->next = nullptr;
        //attach the node
        if (ind == 0) {
            list_maps = tmp;
            list_maps->next = nullptr;
            current_node = list_maps;
        }
        current_node->next = tmp;
        current_node = tmp;
        ind++;
        //printf("%s",buf);
    }

    //close file
    fclose(file);


    //g_last_head=list_maps;
    maps_it->head = list_maps;
    maps_it->current = list_maps;
    return maps_it;
}


procmaps_struct *pmparser_next(procmaps_iterator *p_procmaps_it) {
    if (p_procmaps_it->current == nullptr)
        return nullptr;
    procmaps_struct *p_current = p_procmaps_it->current;
    p_procmaps_it->current = p_procmaps_it->current->next;
    return p_current;
    /*
    if(g_current==NULL){
        g_current=g_last_head;
    }else
        g_current=g_current->next;

    return g_current;
    */
}


void pmparser_free(procmaps_iterator *p_procmaps_it) {
    procmaps_struct *maps_list = p_procmaps_it->head;
    if (maps_list == nullptr) return;
    procmaps_struct *act = maps_list;
    procmaps_struct *nxt = act->next;
    while (act != nullptr) {
        free(act);
        act = nxt;
        if (nxt != nullptr)
            nxt = nxt->next;
    }
    free(p_procmaps_it);
}


void _pmparser_split_line(
        char *buf, char *addr1, char *addr2,
        char *perm, char *offset, char *device, char *inode,
        char *pathname) {
    //
    int orig = 0;
    int i = 0;
    //addr1
    while (buf[i] != '-') {
        addr1[i - orig] = buf[i];
        i++;
    }
    addr1[i] = '\0';
    i++;
    //addr2
    orig = i;
    while (buf[i] != '\t' && buf[i] != ' ') {
        addr2[i - orig] = buf[i];
        i++;
    }
    addr2[i - orig] = '\0';

    //perm
    while (buf[i] == '\t' || buf[i] == ' ')
        i++;
    orig = i;
    while (buf[i] != '\t' && buf[i] != ' ') {
        perm[i - orig] = buf[i];
        i++;
    }
    perm[i - orig] = '\0';
    //offset
    while (buf[i] == '\t' || buf[i] == ' ')
        i++;
    orig = i;
    while (buf[i] != '\t' && buf[i] != ' ') {
        offset[i - orig] = buf[i];
        i++;
    }
    offset[i - orig] = '\0';
    //dev
    while (buf[i] == '\t' || buf[i] == ' ')
        i++;
    orig = i;
    while (buf[i] != '\t' && buf[i] != ' ') {
        device[i - orig] = buf[i];
        i++;
    }
    device[i - orig] = '\0';
    //inode
    while (buf[i] == '\t' || buf[i] == ' ')
        i++;
    orig = i;
    while (buf[i] != '\t' && buf[i] != ' ') {
        inode[i - orig] = buf[i];
        i++;
    }
    inode[i - orig] = '\0';
    //pathname
    pathname[0] = '\0';
    while (buf[i] == '\t' || buf[i] == ' ')
        i++;
    orig = i;
    while (buf[i] != '\t' && buf[i] != ' ' && buf[i] != '\n') {
        pathname[i - orig] = buf[i];
        i++;
    }
    pathname[i - orig] = '\0';

}


uintptr_t getModule(const char *moduleName, bool isStart) {
    int fd = open("/proc/self/maps", O_RDONLY);
    uintptr_t start = 0;
    uintptr_t end = 0;
    bool firstFind = true;
    char line[PATH_MAX];
    char *p = line, *e;
    size_t n = PATH_MAX - 1;
    ssize_t r;
    while ((r = TEMP_FAILURE_RETRY(read(fd, p, n))) > 0) {
        p[r] = '\0';
        p = line; // search begin at line start
        while ((e = strchr(p, '\n')) != nullptr) {
            e[0] = '\0';
            if (strstr(p, moduleName) != nullptr) {
                if (isStart) {
                    //find start
                    if (firstFind) {
                        start = strtoul(p, nullptr, 16);
                        firstFind = false;
                    }
                } else {
#ifdef __LP64__
                    end = strtoul(p + 11, nullptr, 16);
#else
                    end = strtoul(p + 9, NULL, 16);
#endif
                    //find end

                }
            }
            p = e + 1;
        }
        if (p == line) { // !any_entry
            goto __break;
        } //if

        const size_t remain = strlen(p);
        if (remain <= (PATH_MAX / 2)) {
            my_memcpy(line, p, remain * sizeof(p[0]));
        } else {
            my_memmove(line, p, remain * sizeof(p[0]));
        } //if

        p = line + remain;
        n = PATH_MAX - 1 - remain;
    }

    __break:
    close(fd);
    return isStart ? start : end;

}

bool editMemProt(uintptr_t address, int prot) {
    uintptr_t page_start = (address) & (~(PAGE_SIZE - 1));
    return mprotect((void *) page_start, PAGE_SIZE, prot) != -1;
}

int getMemPermission(uintptr_t address) {
    char line[PATH_MAX] = {0};
    char perms[5];
    int bol = 1;

    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == nullptr) {
        return 0;
    }

    while (fgets(line, PATH_MAX, fp) != nullptr) {
        unsigned long start, end;
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


uintptr_t findEGLFunctionPointer(uintptr_t libEGLBase, long size, uintptr_t addr) {
    uintptr_t ret = 0;
    auto *ptr = reinterpret_cast<uintptr_t *>(libEGLBase);
    for (int i = 0; i < (size / sizeof(uintptr_t)); i++) {
        if (!IsPtrValid((void *) ptr[i])) {
            continue;
        }
        if (ptr[i] == addr) {
            auto cret = (uintptr_t) (libEGLBase + i * sizeof(uintptr_t));
            bool isVTable = true;
            for (int j = -10; j < 10; ++j) {
                //在so的内存，包括了bss
                uintptr_t xref = getA2(cret + j * sizeof(void *));
                if (xref > libEGLBase + size || xref < libEGLBase) {
                    isVTable = false;
                    break;
                }
                //可写
                uintptr_t orig = getA2(cret + j * sizeof(void *));
                if (!writePointer3(cret + j * sizeof(void *), orig)) {
                    isVTable = false;
                    break;
                }
            }
            //找到了
            if (isVTable) {
                ret = cret;
            }
        }
    }
    return ret;
}

uintptr_t findEGLFunctionAddress(uintptr_t libEGLBase, long size) {
    uintptr_t ret = 0;
    int *ptr = (int *) libEGLBase;
    for (int i = 0; i < (size / sizeof(int)) - 3; i++) {
        if (ptr[i] == -1440807966 && ptr[i + 1] == 706675683) {
            uintptr_t eglFunAddr = libEGLBase + i * sizeof(int);
            //查找指针
            uintptr_t eglFunPtr = findEGLFunctionPointer(libEGLBase, size, eglFunAddr);
            if (IsPtrValid((void *) eglFunPtr)) {
                ret = eglFunPtr;
                break;
            }
            //未查找到，换函数
            eglFunPtr = findEGLFunctionPointer(libEGLBase, size, eglFunAddr - 1 * sizeof(int));
            if (IsPtrValid((void *) eglFunPtr)) {
                ret = eglFunPtr;
                break;
            }
        }
    }
    return ret;
}

//64位eglSwapBuffers特征
uintptr_t search_eglSwap_addr(uintptr_t begin, uintptr_t end) {
    uintptr_t start = begin;
    uintptr_t limit = end - sizeof(int32_t) * 2;
    do {
        uintptr_t base_addr = getA2(start);
        if (IsPtrValid((void *) base_addr)) {
            auto *insn = reinterpret_cast<int *>(base_addr);
            auto *insn2 = reinterpret_cast<int *>(base_addr - 0x20);
            if ((insn[0] == 0xaa1f03e2 && insn[1] == 0x2a1f03e3 && insn2[0] == 0xaa1403e1) ||
                (insn[0] == 0xaa1f03e2 && insn[1] == 0x2a1f03e3 && insn2[1] == 0xaa1403e1) ||
                (insn[1] == 0xaa1f03e2 && insn[2] == 0x2a1f03e3 && insn2[1] == 0xaa1403e1)) {
                return start;
            }
        }
        start += sizeof(int32_t);
    } while (start < limit);
    return 0;
}

uintptr_t findEGLFun() {
    uintptr_t ret = -1;
    procmaps_iterator *maps = pmparser_parse(-1);
    if (maps != nullptr) {
        procmaps_struct *maps_tmp;
        while ((maps_tmp = pmparser_next(maps)) != nullptr) {
            if (strstr(maps_tmp->pathname, "libEGL.so")) {
                if (maps_tmp->is_r && maps_tmp->is_w) {
                    ret = search_eglSwap_addr((uintptr_t) maps_tmp->addr_start, (uintptr_t) maps_tmp->addr_end);
                }
            }
            if (ret == 0) {
                if (strstr(maps_tmp->pathname, "anon:.bss")) {
                    if (maps_tmp->is_r && maps_tmp->is_w) {
                        ret = search_eglSwap_addr((uintptr_t) maps_tmp->addr_start, (uintptr_t) maps_tmp->addr_end);
                        //只查找libEGL下一段的bss
                        if (ret == 0) {
                            ret = -1;
                        }
                    }

                }
            }
        }
    }
    pmparser_free(maps);
    return ret;
}

uintptr_t findEGLFunctionUE4() {
    uintptr_t ret = 0;
    procmaps_iterator *maps = pmparser_parse(-1);
    if (maps == nullptr) {
        return 0;
    }
    procmaps_struct *maps_tmp;
    while ((maps_tmp = pmparser_next(maps)) != nullptr) {
        if (strcmp(basename(maps_tmp->pathname), "libUE4.so") == 0 && maps_tmp->is_w) {
            uintptr_t base = (uintptr_t) maps_tmp->addr_start;
            int size = (uintptr_t) maps_tmp->addr_end - base - 1;
            uintptr_t *ptr = (uintptr_t *) base;
            for (int i = 0; i < (size / sizeof(uintptr_t)); i++) {
                if (ptr[i] == (uintptr_t) eglSwapBuffers) {
                    ret = base + i * sizeof(uintptr_t);
                    break;
                }
            }
        }
    }
    pmparser_free(maps);
    return ret;
}

#pragma clang diagnostic pop