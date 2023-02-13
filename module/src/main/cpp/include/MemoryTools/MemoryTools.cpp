#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"

#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/uio.h>
#include <dirent.h>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <ctime>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "MemoryTools.h"
#include "MapTools.h"
#include "bionic_netlink.h"
#include <sys/stat.h>
#include <searchCall.h>
#include <elf_util.h>
#include <sys/prctl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/ptrace.h>

using namespace std;

#define AARCH64_SVC_0 0xD4000001

string getMacAddress() {
    ifaddrs *ifap, *ifaptr;
    if (myGetifaddrs(&ifap) == 0) {
        for (ifaptr = ifap; ifaptr != nullptr; ifaptr = (ifaptr)->ifa_next) {
            if(ifaptr->ifa_addr!= nullptr) {
                if (((ifaptr)->ifa_addr)->sa_family == AF_PACKET) {
                    if (strcmp(ifaptr->ifa_name,"wlan0") == 0) {
                        auto *sockadd = (struct sockaddr_ll *) (ifaptr->ifa_addr);
                        int len = 0;
                        char macp[INET6_ADDRSTRLEN];
                        for (int i = 0; i < 6; i++) {
                            len += sprintf(macp + len, "%02X%s", sockadd->sll_addr[i],( i < 5 ? ":" : ""));
                        }
                        freeifaddrs(ifap);
                        return {macp};
                    }
                }
            }

        }
        freeifaddrs(ifap);
    }
    return "";
}

void *my_memmove_fun(void *dst, const void *src, size_t count) {
    // 容错处理
    if (src == nullptr || dst == nullptr) {
        return nullptr;
    }
    auto *pdst = (unsigned char *) dst;
    const auto *psrc = (const unsigned char *) src;
    if (psrc == nullptr) {
        return nullptr;
    }
    //判断内存是否重叠
    bool flag1 = (pdst >= psrc && pdst < psrc + count);
    bool flag2 = (psrc >= pdst && psrc < pdst + count);
    if (flag1 || flag2) {
        // 倒序拷贝
        while (count) {
            *(pdst + count - 1) = *(psrc + count - 1);
            count--;
        }//while
    } else {
        // 拷贝
        while (count--) {
            *pdst = *psrc;
            pdst++;
            psrc++;
        }//while
    }
    return dst;
}

void *my_memmove(void *dst, const void *src, size_t count) {
    if (src == nullptr || dst == nullptr) {
        return nullptr;
    }
    char *tmp_dst = (char *) dst;
    char *tmp_src = (char *) src;
    if (tmp_src == nullptr) {
        return nullptr;
    }
    if (tmp_dst + count < src || tmp_src + count < dst) {
        while (count--)
            *tmp_dst++ = *tmp_src++;
    } else {
        tmp_dst += count - 1;
        tmp_src += count - 1;
        while (count--)
            *tmp_dst-- = *tmp_src--;
    }

    return dst;
}

void *my_memcpy(void *dst, const void *src, size_t count) {
    if (src == nullptr || dst == nullptr) {
        return nullptr;
    }
    char *tmp_dst = (char *) dst;
    char *tmp_src = (char *) src;
    if (tmp_src == nullptr) {
        return nullptr;
    }
    while (count--)
        *tmp_dst++ = *tmp_src++;

    return dst;
}

char *my_strcpy(char *dest, const char *src) {
    //assert((dest != NULL) && (src != NULL));
    if (src == nullptr || dest == nullptr) {
        return nullptr;
    }
    const char *end = src;
    while (*end)
        end++;
    my_memcpy(dest, src, end - src + 1);
    return dest;
}


bool memoryRead(uintptr_t address, void *buffer, int size) {
    memset(buffer, 0, size);
    if (address <= 0x10000000 || address >= 0x10000000000)
        return false;
    if (!isSafeAddress(address, size))
        return false;
    return my_memmove(buffer, reinterpret_cast<void *>(address), size) != nullptr;
}

static int random_fd = -1;

bool IsPtrValid(void *addr) {
    if (!addr)
        return false;
    if (random_fd <= 0)
        random_fd = raw_syscall(__NR_memfd_create, "jit-cache",MFD_CLOEXEC | MFD_ALLOW_SEALING);
    return raw_syscall(__NR_write, random_fd, addr, 4) >= 0;
}


bool isSafeAddress(uintptr_t _addr, size_t _size) {
    return IsPtrValid(reinterpret_cast<void *>(_addr));
}

static pid_t pid = -1;
bool process_v(uintptr_t address, void *buffer, size_t size, bool iswrite) {
    if (pid < 0)
        pid = getpid();
    struct iovec local{}, remote{};
    local.iov_base = buffer;
    local.iov_len = size;
    remote.iov_base = reinterpret_cast<void *>(address);
    remote.iov_len = size;
    return raw_syscall(iswrite ? __NR_process_vm_writev : __NR_process_vm_readv, pid, &local, 1, &remote, 1, 0) == size;
}

int getI(uintptr_t address) {
    int value = 0;
    if (!memoryRead(address, &value, sizeof(int))) {
        return false;
    }
    return value;
}

uintptr_t getA(uintptr_t address) {
    uintptr_t value = 0;
    if (!memoryRead(address, &value, sizeof(uintptr_t))) {
        return false;
    }
    return value;
}

uintptr_t getA2(uintptr_t address) {
    uintptr_t value = 0;
    if (!process_v(address, &value, sizeof(uintptr_t),false)) {
        return false;
    }
    return value;
}

float getF(uintptr_t address) {
    float value = 0.0f;
    if (!memoryRead(address, &value, sizeof(float))) {
        return false;
    }
    return value;
}

bool writePointer3(uintptr_t addr,uintptr_t value) {
    return process_v(addr, &value, sizeof(value),true);
}

char getChar(uintptr_t address) {
    char value = 0;
    if (!memoryRead(address, &value, sizeof(char))) {
        return false;
    }
    return value;
}


void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst) {
    std::string::size_type pos = 0;
    std::string::size_type srclen = strsrc.size();
    std::string::size_type dstlen = strdst.size();
    while((pos=strBig.find(strsrc, pos)) != std::string::npos) {
        strBig.replace(pos, srclen, strdst);
        pos += dstlen;
    }
}

void writeBuffer(uintptr_t address, void *buffer, int size) {
    if (address <= 0x10000000 || address >= 0x10000000000)
        return ;
    //修改内存属性
    int prot = getMemPermission(address);
    if (prot == 0)
        return;
    if (!isSafeAddress(address, size))
        return;
    if (!editMemProt(address, PROT_READ | PROT_WRITE | PROT_EXEC)) // 修改内存段的权限属性 -- 可读|可写|可执行
        return;
    my_memmove((void *) address, buffer, size);
    editMemProt(address, prot);//还原内存权限
}

void writeFloat(uintptr_t address, float value) {
    if (getF(address) != value) {
        writeBuffer(address, &value, sizeof(value));
    }
}

void writeInt(uintptr_t address, int value) {
    if (getI(address) != value) {
        writeBuffer(address, &value, sizeof(value));
    }
}

void writePointer(uintptr_t address, uintptr_t value) {
    if (getA(address) != value) {
        writeBuffer(address, &value, sizeof(value));
    }
}

#pragma clang diagnostic pop