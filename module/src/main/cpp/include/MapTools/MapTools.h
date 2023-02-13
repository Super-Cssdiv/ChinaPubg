#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <cinttypes>
#include <set>
#include <string_view>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <errno.h>
#include <malloc.h>

// User lib
#include <Logger.h>
#define PROCMAPS_LINE_MAX_LENGTH  (PATH_MAX + 100)

typedef struct procmaps_struct{
    uintptr_t addr_start; 	//< start address of the area
    uintptr_t addr_end; 	//< end address
    intptr_t length; //< size of the range

    char perm[5];		//< permissions rwxp
    short is_r;			//< rewrote of perm with short flags
    short is_w;
    short is_x;
    short is_p;

    long offset;	//< offset
    char dev[12];	//< dev major:minor
    int inode;		//< inode of the file that backs the area

    char pathname[600];		//< the path of the file that backs the area
    //chained list
    struct procmaps_struct* next;		//<handler of the chinaed list
} procmaps_struct;

typedef struct procmaps_iterator{
    procmaps_struct* head;
    procmaps_struct* current;
} procmaps_iterator;

procmaps_iterator* pmparser_parse(int pid);

procmaps_struct* pmparser_next(procmaps_iterator* p_procmaps_it);

void pmparser_free(procmaps_iterator* p_procmaps_it);

void _pmparser_split_line(char*buf,char*addr1,char*addr2,char*perm, char* offset, char* device,char*inode,char* pathname);

uintptr_t getModule(const char *lib, bool isStart);

bool editMemProt(uintptr_t address, int prot);

int getMemPermission(uintptr_t address);

uintptr_t findEGLFunctionPointer(uintptr_t libEGLBase, long size, uintptr_t addr);

uintptr_t findEGLFunctionAddress(uintptr_t libEGLBase, long size);

uintptr_t search_eglSwap_addr(uintptr_t begin, uintptr_t end);

uintptr_t findEGLFun();

uintptr_t findEGLFunctionUE4();