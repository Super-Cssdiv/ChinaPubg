#pragma once


#include <stdint.h>
#define AARCH64_SVC_0 0xD4000001
#define AARCH64_IS_MOV(insn) ((int32_t)((insn) & 0xFFE0001F) == 0xD2800008)
#define ARM_IS_MOV_R7_IMM(insn) (((insn) & 0xFF00F000) == 0xE3007000)

namespace searchCall {
    uintptr_t findSyscalls64(const char *path, long num);
    uintptr_t findSyscalls32(const char *path, int num) ;
}

