
#include <cstdlib>
#include "inject.h"
InjectTools *InjectTools::sInjectTools = nullptr;

InjectTools *InjectTools::getInstance(pid_t pid) {
    if(!sInjectTools){
        sInjectTools = new InjectTools(pid);
    }
    return sInjectTools;
}


bool InjectTools::injectStart( ) {
    // attach到目标进程
    if (ptrace_attach(pid) != 0) {
        return false;
    }

    // CurrentRegs 当前寄存器
    // OriginalRegs 保存注入前寄存器
    if (ptrace_getregs(pid, &CurrentRegs) != 0) {
        //进程不存在
        return false;
    }
    // 保存原始寄存器
    memcpy(&OriginalRegs, &CurrentRegs, sizeof(CurrentRegs));

    mmapAddr = get_mmap_address(pid);
    munmapAddr = get_munmap_address(pid);
    dlopenAddr = get_dlopen_address(pid);
    dlsymAddr = get_dlsym_address(pid);
    dlcloseAddr = get_dlclose_address(pid);

    mprotectAddr = get_mprotect_address(pid);
    memcpyAddr = get_memcpy_address(pid);
    syscallAddr = get_syscall_address(pid);
    ftruncateAddr = get_ftruncate_address(pid);
    writeAddr = get_write_address(pid);
    closeAddr = get_close_address(pid);
    return true;
}

bool InjectTools::injectEnd( ) {
    //设置规则
    if (ptrace_setregs(pid, &OriginalRegs) == -1) {
        //设置规则错误
        return false;
    }
    // 解除attach
    ptrace_detach(pid);
    return true;
}

void *InjectTools::callMmap(void *addr, size_t size, int prot, int flags, int fd, off_t offset) {
    uintptr_t parameters[6];
    parameters[0] = (uintptr_t) addr; // 设置为NULL表示让系统自动选择分配内存的地址
    parameters[1] = size; // 映射内存的大小
    parameters[2] = prot; // 表示映射内存区域 可读|可写|可执行
    parameters[3] = flags; // 建立匿名映射
    parameters[4] = fd; //  若需要映射文件到内存中，则为文件的fd
    parameters[5] = offset; //文件映射偏移量
    // 调用远程进程的mmap函数申请内存
    if (ptrace_call(pid, (uintptr_t) mmapAddr, parameters, 6, &CurrentRegs) == -1) {
        return nullptr;
    }
    // 获取mmap函数执行后的返回值
    return (void *) ptrace_getret(&CurrentRegs);
}

bool InjectTools::writeRemoteMemory(void *remoteAddr, void *buffer, size_t size) {
    if (lastSize != 0) {
        void *tmpMemory = malloc(size);
        memset(tmpMemory, 0, size);
        if (ptrace_writedata(pid, (uint8_t *) remoteAddr, (uint8_t *) tmpMemory, size) == -1) {
            return false;
        }

    }
    if (ptrace_writedata(pid, (uint8_t *) remoteAddr, (uint8_t *) buffer, size) == -1) {
        return false;
    }
    lastSize = size;

    return true;
}

bool InjectTools::callMunmap(void *remoteAddr, size_t size) {
    uintptr_t parameters[2];
    parameters[0] = (uintptr_t) remoteAddr; //申请的内存区域地址头
    parameters[1] = size;
    // 调用远程进程的munmap函数 卸载内存
    if (ptrace_call(pid, (uintptr_t) munmapAddr, parameters, 2, &CurrentRegs) == -1) {
        return false;
    }
    return true;
}

void *InjectTools::callDlOpen(void *remotePathAddr, int mode) {
    uintptr_t parameters[2];
    parameters[0] = (uintptr_t) remotePathAddr;
    parameters[1] = mode;
    if (ptrace_call(pid, (uintptr_t) dlopenAddr, parameters, 2, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

bool InjectTools::callDlClose(void *remoteHandle) {
    uintptr_t parameters[1] = {(uintptr_t) remoteHandle};
    if (ptrace_call(pid, (uintptr_t) dlcloseAddr, parameters, 1, &CurrentRegs) == -1) {
        return false;
    }
    return true;
}

void *InjectTools::findFunction(void *remoteLibAddr, void *remoteFunctionName) {
    uintptr_t parameters[2];
    parameters[0] = (uintptr_t) remoteLibAddr;
    parameters[1] = (uintptr_t) remoteFunctionName;
    if (ptrace_call(pid, (uintptr_t) dlsymAddr, parameters, 2, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

void *InjectTools::callCustomFunction(void *remoteFuncitonAddr, void *remoteParameters, int parameterSize) {
    if (ptrace_call(pid, (uintptr_t) remoteFuncitonAddr, (uintptr_t *) remoteParameters, parameterSize, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

void *InjectTools::callMprotect(void *remoteAddr, size_t size, int prot) {
    uintptr_t parameters[3];
    parameters[0] = (uintptr_t) remoteAddr;
    parameters[1] = size;
    parameters[2] = prot;
    if (ptrace_call(pid, (uintptr_t) mprotectAddr, parameters, 3, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

void *InjectTools::callMemcpy(void *remoteDst, void *remoteSrc, size_t srcSize) {
    uintptr_t parameters[3];
    parameters[0] = (uintptr_t) remoteDst;
    parameters[1] = (uintptr_t) remoteSrc;
    parameters[2] = srcSize;
    if (ptrace_call(pid, (uintptr_t) memcpyAddr, parameters, 3, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

void *InjectTools::callSyscall(void *parameters, int parameterSize) {
    if (ptrace_call(pid, (uintptr_t) syscallAddr, (uintptr_t *) parameters, parameterSize, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

void *InjectTools::callFtruncate(int fd, size_t size) {
    uintptr_t parameters[2];
    parameters[0] = fd;
    parameters[1] = size;
    if (ptrace_call(pid, (uintptr_t) ftruncateAddr, parameters, 2, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

void *InjectTools::callWrite(int fd, void *buffer, size_t size) {
    uintptr_t parameters[3];
    parameters[0] = fd;
    parameters[1] = (uintptr_t) buffer;
    parameters[2] = size;
    if (ptrace_call(pid, (uintptr_t) writeAddr, parameters, 3, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

void *InjectTools::callClose(int fd) {
    uintptr_t parameters[1];
    parameters[0] = fd;
    if (ptrace_call(pid, (uintptr_t) closeAddr, parameters, 1, &CurrentRegs) == -1) {
        return nullptr;
    }
    return (void *) ptrace_getret(&CurrentRegs);
}

