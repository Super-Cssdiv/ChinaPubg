#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include "elf_util.h"

struct soinfo {
    const void *phdr;
    size_t phnum;
    uint64_t base;
    size_t size;
    void *dynamic;
    soinfo *next;
};

class HideTools {
private:
    static const char * get_realpath(soinfo *);
    static soinfo *solist_get_head();
    static soinfo *solist_get_next();
    static bool solist_remove_soinfo(soinfo* si);
    static int getMemPermission(uint64_t address);
public:
     static std::vector<const char *> GetSoinfoList();
     static bool HideSoinfo(const std::string &name);
};

