#ifndef _COMMON
#define _COMMON

#include <string>

extern "C" long raw_syscall(long __number, ...);

/* 读取模块入口地址 */
char *my_strcpy(char *dest, const char *src);
void *my_memmove(void *dst, const void *src, size_t count);
void *my_memcpy(void *dst, const void *src, size_t count);

void writeBuffer(uintptr_t address, void *buffer, int size);
//缺页判断
bool isSafeAddress(uintptr_t _addr, size_t _size);

//内存读取
bool memoryRead(uintptr_t address, void *buffer, int size);
float getF(uintptr_t address);
uintptr_t getA(uintptr_t address);
char getChar(uintptr_t address);
int getI(uintptr_t address);
void writeFloat(uintptr_t address, float value);
void writeInt(uintptr_t address, int value);
bool writePointer3(uintptr_t addr,uintptr_t value);
bool process_v(uintptr_t address, void *buffer, size_t size, bool iswrite);

std::string getMacAddress();

bool IsPtrValid(void *addr);


void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);
void *my_memmove(void *dst, const void *src, size_t count);
void writePointer(uintptr_t address, uintptr_t value);
uintptr_t getA2(uintptr_t address);
bool install_filter(uint32_t num, int32_t error);
bool pass_seccomp();
#endif