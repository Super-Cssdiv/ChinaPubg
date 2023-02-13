#pragma once
// system lib
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <elf.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
// user lib
#include "ptrace_utils.h"

class InjectTools {
private:
    pid_t pid;
    void *mmapAddr;
    void *munmapAddr;
    void *dlopenAddr;
    void *dlsymAddr;
    void *dlcloseAddr;
    void *mprotectAddr;
    void *memcpyAddr;
    void *syscallAddr;
    void *ftruncateAddr;
    void *writeAddr;
    void *closeAddr;
    struct pt_regs CurrentRegs, OriginalRegs;

    size_t lastSize = 0;

    static InjectTools *sInjectTools;
public:
    InjectTools(pid_t pid) {
        if(pid == -1){
            pid = getpid();
        }
        this->pid = pid;
    }

    static InjectTools *getInstance(pid_t pid);

    bool injectStart( );

    bool injectEnd( );

    void *callMmap(void *addr, size_t size, int prot, int flags, int fd, off_t offset);

    bool writeRemoteMemory(void *remoteAddr, void *buffer, size_t size);

    bool callMunmap(void *remoteAddr, size_t size);

    void *callDlOpen(void *remotePathAddr, int mode);

    bool callDlClose(void *remoteHandle);

    void *findFunction(void *remoteLibAddr, void *remoteFunctionName);

    void *callCustomFunction(void *remoteFuncitonAddr, void *remoteParameters, int parameterSize);

    void *callMprotect(void *remoteAddr, size_t size, int prot);

    void *callMemcpy(void *remoteDst, void *remoteSrc, size_t srcSize);

    void *callSyscall(void *parameters, int parameterSize);

    void *callFtruncate(int fd, size_t size);

    void *callWrite(int fd, void *buffer, size_t size);

    void *callClose(int fd);
};

