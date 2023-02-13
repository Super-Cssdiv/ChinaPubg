//
// Created by Administrator on 2022/9/25.
//

#include <searchCall.h>
#include <Logger.h>
#include <MapTools.h>

static uintptr_t search_memory_syscall64(uintptr_t begin, uintptr_t end, long num) {
    uintptr_t start = begin;
    uintptr_t limit = end - sizeof(int32_t) * 2;
    do {
        auto *insn = reinterpret_cast<int32_t *>(start);
        if (insn[1] == AARCH64_SVC_0 && AARCH64_IS_MOV(insn[0])) {
            long syscall_num = (unsigned) ((insn[0] >> 5) & 0xFFFF);
            if (syscall_num == num) {
                return start;
            }
        }
        start += sizeof(int32_t);
    } while (start < limit);
    return 0;
}


uintptr_t searchCall::findSyscalls64(const char *path, long num) {
    uintptr_t base = 0;
    procmaps_iterator *maps = pmparser_parse(-1);
    if (maps == nullptr) {
        return 0;
    }
    procmaps_struct *maps_tmp;
    while ((maps_tmp = pmparser_next(maps)) != nullptr) {
        if (strstr(maps_tmp->pathname, path)) {
            if (maps_tmp->is_r && maps_tmp->is_x) {
                base = search_memory_syscall64(maps_tmp->addr_start, maps_tmp->addr_end, num);
                if (base != 0) {
                    break;
                }
            }
        }
    }
    pmparser_free(maps);
    return base;
}

static uintptr_t search_memory_syscall32(uintptr_t begin, uintptr_t end,int num) {
    uintptr_t addr = 0;
    uintptr_t start = begin;
    uintptr_t limit = end - sizeof(int32_t) * 4;
    do {
        auto *insn = reinterpret_cast<int32_t *>(start);
        if (insn[0] == 0xE1A0C007 && ARM_IS_MOV_R7_IMM(insn[1]) && insn[2] == 0xEF000000) {
            int32_t value = insn[1];
            int syscall = ((value & 0xF0000) >> 4) | (value & 0x00FFF);
            if (syscall == num) {
                addr = start;
            }
        }
        start += 1;
    } while (start < limit);
    return addr;
}

uintptr_t searchCall::findSyscalls32(const char *path, int num) {
    uintptr_t base = 0;
    procmaps_iterator *maps = pmparser_parse(-1);
    if (maps == nullptr) {
        return 0;
    }
    procmaps_struct *maps_tmp;
    while ((maps_tmp = pmparser_next(maps)) != nullptr) {
        if (strstr(maps_tmp->pathname, path)) {
            if (maps_tmp->is_r && maps_tmp->is_x) {
                base = search_memory_syscall32(maps_tmp->addr_start, maps_tmp->addr_end, num);
                if (base != 0) {
                    break;
                }
            }
        }
    }
    pmparser_free(maps);
    return base;
}

