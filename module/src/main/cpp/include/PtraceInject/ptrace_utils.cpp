/**
 * 让Ptrace注入兼容多平台的主要步骤在这里
 */
#include <cstdlib>
#include <sys/system_properties.h>
#include "ptrace_utils.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#pragma clang diagnostic ignored "-Wformat"
// 系统lib路径
struct ProcessLibs process_libs = {"", "", ""};
/**
 * @brief 处理各架构预定义的库文件
 */

void setLibcPath(char* libc_path) {
    strcpy(process_libs.libc_path, libc_path);
}

void setLinkerPath(char* linker_path) {
    strcpy(process_libs.linker_path, linker_path);
}

void setLibdlPath(char* libdl_path) {
    strcpy(process_libs.libdl_path, libdl_path);
}

pid_t get_pid_by_name(const char *task_name) {
    int id;
    pid_t pid = -1;
    DIR *dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];

    struct dirent *entry;

    if (task_name == NULL) {
        return -1;
    }

    dir = opendir("/proc");
    if (dir == NULL) {
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id != 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);

                if (strcmp(task_name, cmdline) == 0) {
                    /* process found */
                    pid = id;
                    break;
                }
            }
        }
    }

    closedir(dir);
    return pid;
}


void *get_module_base_addr(pid_t pid, const char *ModuleName) {
    FILE *fp = NULL;
    long ModuleBaseAddr = 0;
    char szFileName[50] = {0};
    char szMapFileLine[1024] = {0};

    // 读取"/proc/pid/maps"可以获得该进程加载的模块
    if (pid < 0) {
        //  枚举自身进程模块
        snprintf(szFileName, sizeof(szFileName), "/proc/self/maps");
    } else {
        snprintf(szFileName, sizeof(szFileName), "/proc/%d/maps", pid);
    }

    fp = fopen(szFileName, "r");

    if (fp != NULL) {
        while (fgets(szMapFileLine, sizeof(szMapFileLine), fp)) {
            if (strstr(szMapFileLine, ModuleName)) {
                char *Addr = strtok(szMapFileLine, "-");
                ModuleBaseAddr = strtoul(Addr, NULL, 16);

                if (ModuleBaseAddr == 0x8000) {
                    ModuleBaseAddr = 0;
                }

                break;
            }
        }

        fclose(fp);
    }

    return (void *) ModuleBaseAddr;
}

void *get_remote_func_addr(pid_t pid, const char *ModuleName, void *LocalFuncAddr) {
    void *LocalModuleAddr, *RemoteModuleAddr, *RemoteFuncAddr;
    //获取本地某个模块的起始地址
    LocalModuleAddr = get_module_base_addr(-1, ModuleName);
    //获取远程pid的某个模块的起始地址
    RemoteModuleAddr = get_module_base_addr(pid, ModuleName);
    // local_addr - local_handle的值为指定函数(如mmap)在该模块中的偏移量，然后再加上remote_handle，结果就为指定函数在目标进程的虚拟地址
    RemoteFuncAddr = (void *) ((uintptr_t) LocalFuncAddr - (uintptr_t) LocalModuleAddr + (uintptr_t) RemoteModuleAddr);

    //printf("[+] [get_remote_func_addr] lmod=0x%lX, rmod=0x%lX, lfunc=0x%lX, rfunc=0x%lX\n", LocalModuleAddr, RemoteModuleAddr, LocalFuncAddr, RemoteFuncAddr);
    return RemoteFuncAddr;
}


int ptrace_attach(pid_t pid) {
    int status = 0;
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        printf("[-] 附加进程失败, pid:%d, err:%s\n", pid, strerror(errno));
        return -1;
    }

    printf("[+] 附加进程成功, pid:%d\n", pid);
    waitpid(pid, &status, WUNTRACED);

    return 0;
}

int ptrace_continue(pid_t pid) {
    if (ptrace(PTRACE_CONT, pid, NULL, NULL) < 0) {
        printf("[-] 恢复进程失败, pid:%d, err:%ss\n", pid, strerror(errno));
        return -1;
    }

   // printf("[+] ptrace continue process success, pid:%d\n", pid);
    return 0;
}

int ptrace_detach(pid_t pid) {
    if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {
        printf("[-] 分离进程失败, pid:%d, err:%s\n", pid, strerror(errno));
        return -1;
    }

    //printf("[+] detach process success, pid:%d\n", pid);
    return 0;
}

int ptrace_getregs(pid_t pid, struct pt_regs *regs) {
#if defined(__aarch64__)
    int regset = NT_PRSTATUS;
    struct iovec ioVec{};

    ioVec.iov_base = regs;
    ioVec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_GETREGSET, pid, (void *) regset, &ioVec) < 0) {
        printf("[-] ptrace_getregs: Can not get register values, io %llx, %d\n", ioVec.iov_base, ioVec.iov_len);
        return -1;
    }

    return 0;
#else
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {
        printf("[-] Get Regs error, pid:%d, err:%s\n", pid, strerror(errno));
        return -1;
    }
#endif
    return 0;
}

int ptrace_setregs(pid_t pid, struct pt_regs *regs) {
#if defined(__aarch64__)
    int regset = NT_PRSTATUS;
    struct iovec ioVec{};

    ioVec.iov_base = regs;
    ioVec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_SETREGSET, pid, (void *) regset, &ioVec) < 0) {
        perror("[-] ptrace_setregs: Can not get register values");
        return -1;
    }

    return 0;
#else
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
        printf("[-] Set Regs error, pid:%d, err:%s\n", pid, strerror(errno));
        return -1;
    }
#endif
    return 0;
}

uintptr_t ptrace_getret(struct pt_regs *regs) {
#if defined(__i386__) || defined(__x86_64__) // 模拟器&x86_64
    return regs->eax;
#elif defined(__arm__) || defined(__aarch64__) // 真机
    return regs->ARM_r0;
#else
    printf("Not supported Environment %s\n", __FUNCTION__);
#endif
}

uintptr_t ptrace_getpc(struct pt_regs *regs) {
#if defined(__i386__) || defined(__x86_64__)
    return regs->eip;
#elif defined(__arm__) || defined(__aarch64__)
    return regs->ARM_pc;
#else
    printf("Not supported Environment %s\n", __FUNCTION__);
#endif
}

void *get_mmap_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) mmap);
}


void *get_munmap_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) munmap);
}

void *get_mprotect_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) mprotect);
}

void *get_memcpy_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) memcpy);
}

void *get_syscall_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) syscall);
}

void *get_ftruncate_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) ftruncate);
}

void *get_write_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) write);
}

void *get_close_address(pid_t pid) {
    return get_remote_func_addr(pid, process_libs.libc_path, (void *) close);
}

void *get_dlopen_address(pid_t pid) {
    void *dlopen_addr;
    char sdk_ver[32];
    memset(sdk_ver, 0, sizeof(sdk_ver));
    __system_property_get("ro.build.version.sdk", sdk_ver);

    //printf("[+] linker_path value:%s\n", process_libs.linker_path);
    if (atoi(sdk_ver) <= 23) { // 安卓7
        dlopen_addr = get_remote_func_addr(pid, process_libs.linker_path, (void *) dlopen);
    } else {
        dlopen_addr = get_remote_func_addr(pid, process_libs.libdl_path, (void *) dlopen);
    }
    //printf("[+] dlopen RemoteFuncAddr:0x%lx\n", (uintptr_t) dlopen_addr);
    return dlopen_addr;
}

void *get_dlclose_address(pid_t pid) {
    void *dlclose_addr;
    char sdk_ver[32];
    memset(sdk_ver, 0, sizeof(sdk_ver));
    __system_property_get("ro.build.version.sdk", sdk_ver);

    if (atoi(sdk_ver) <= 23) {
        dlclose_addr = get_remote_func_addr(pid, process_libs.linker_path, (void *) dlclose);
    } else {
        dlclose_addr = get_remote_func_addr(pid, process_libs.libdl_path, (void *) dlclose);
    }
    return dlclose_addr;
}

void *get_dlsym_address(pid_t pid) {
    void *dlsym_addr;
    char sdk_ver[32];
    memset(sdk_ver, 0, sizeof(sdk_ver));
    __system_property_get("ro.build.version.sdk", sdk_ver);

    if (atoi(sdk_ver) <= 23) {
        dlsym_addr = get_remote_func_addr(pid, process_libs.linker_path, (void *) dlsym);
    } else {
        dlsym_addr = get_remote_func_addr(pid, process_libs.libdl_path, (void *) dlsym);
    }
    //printf("[+] dlsym RemoteFuncAddr:0x%lx\n", (uintptr_t) dlsym_addr);
    return dlsym_addr;
}

void *get_dlerror_address(pid_t pid) {
    void *dlerror_addr;
    char sdk_ver[32];
    memset(sdk_ver, 0, sizeof(sdk_ver));
    __system_property_get("ro.build.version.sdk", sdk_ver);

    if (atoi(sdk_ver) <= 23) {
        dlerror_addr = get_remote_func_addr(pid, process_libs.linker_path, (void *) dlerror);
    } else {
        dlerror_addr = get_remote_func_addr(pid, process_libs.libdl_path, (void *) dlerror);
    }
    //printf("[+] dlerror RemoteFuncAddr:0x%lx\n", (uintptr_t) dlerror_addr);
    return dlerror_addr;
}

int ptrace_readdata(pid_t pid, uint8_t *pSrcBuf, uint8_t *pDestBuf, size_t size) {
    uintptr_t nReadCount = 0;
    uintptr_t nRemainCount = 0;
    uint8_t *pCurSrcBuf = pSrcBuf;
    uint8_t *pCurDestBuf = pDestBuf;
    uintptr_t lTmpBuf = 0;
    uintptr_t i = 0;

    nReadCount = size / sizeof(uintptr_t);
    nRemainCount = size % sizeof(uintptr_t);

    for (i = 0 ; i < nReadCount ; i++) {
        lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurSrcBuf, 0);
        memcpy(pCurDestBuf, (char *) (&lTmpBuf), sizeof(uintptr_t));
        pCurSrcBuf += sizeof(uintptr_t);
        pCurDestBuf += sizeof(uintptr_t);
    }

    if (nRemainCount > 0) {
        lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurSrcBuf, 0);
        memcpy(pCurDestBuf, (char *) (&lTmpBuf), nRemainCount);
    }

    return 0;
}

int ptrace_writedata(pid_t pid, uint8_t *pWriteAddr, uint8_t *pWriteData, size_t size) {

    uintptr_t nWriteCount = 0;
    uintptr_t nRemainCount = 0;
    uint8_t *pCurSrcBuf = pWriteData;
    uint8_t *pCurDestBuf = pWriteAddr;
    uintptr_t lTmpBuf = 0;
    uintptr_t i = 0;

    nWriteCount = size / sizeof(uintptr_t);
    nRemainCount = size % sizeof(uintptr_t);

    // 先讲数据以sizeof(uintptr_t)字节大小为单位写入到远程进程内存空间中
    for (i = 0 ; i < nWriteCount ; i++) {
        memcpy((void *) (&lTmpBuf), pCurSrcBuf, sizeof(uintptr_t));
        if (ptrace(PTRACE_POKETEXT, pid, (void *) pCurDestBuf, (void *) lTmpBuf) < 0) { // PTRACE_POKETEXT表示从远程内存空间写入一个sizeof(uintptr_t)大小的数据
            printf("[-] Write Remote Memory error, MemoryAddr:0x%lx, err:%s\n", (uintptr_t) pCurDestBuf, strerror(errno));
            return -1;
        }
        pCurSrcBuf += sizeof(uintptr_t);
        pCurDestBuf += sizeof(uintptr_t);
    }
    // 将剩下的数据写入到远程进程内存空间中
    if (nRemainCount > 0) {
        lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurDestBuf, NULL); //先取出原内存中的数据，然后将要写入的数据以单字节形式填充到低字节处
        memcpy((void *) (&lTmpBuf), pCurSrcBuf, nRemainCount);
        if (ptrace(PTRACE_POKETEXT, pid, pCurDestBuf, lTmpBuf) < 0) {
            printf("[-] Write Remote Memory error, MemoryAddr:0x%lx, err:%s\n", (uintptr_t) pCurDestBuf, strerror(errno));
            return -1;
        }
    }
    return 0;
}

int ptrace_call(pid_t pid, uintptr_t ExecuteAddr, uintptr_t *parameters, uintptr_t num_params, struct pt_regs *regs) {
#if defined(__i386__) // 模拟器
    // 写入参数到堆栈
    regs->esp -= (num_params) * sizeof(uintptr_t); // 分配栈空间，栈的方向是从高地址到低地址
    if (0 != ptrace_writedata(pid, (uint8_t *)regs->esp, (uint8_t *)parameters,(num_params) * sizeof(uintptr_t))){
        return -1;
    }

    uintptr_t tmp_addr = 0x0;
    regs->esp -= sizeof(uintptr_t);
    if (0 != ptrace_writedata(pid, (uint8_t *)regs->esp, (uint8_t *)&tmp_addr, sizeof(tmp_addr))){
        return -1;
    }

    //设置eip寄存器为需要调用的函数地址
    regs->eip = ExecuteAddr;

    // 开始执行
    if (-1 == ptrace_setregs(pid, regs) || -1 == ptrace_continue(pid)){
        printf("[-] ptrace set regs or continue error, pid:%d\n", pid);
        return -1;
    }

    int stat = 0;
    // 对于使用ptrace_cont运行的子进程，它会在3种情况下进入暂停状态：①下一次系统调用；②子进程退出；③子进程的执行发生错误。
    // 参数WUNTRACED表示当进程进入暂停状态后，立即返回
    waitpid(pid, &stat, WUNTRACED);

    // 判断是否成功执行函数
    printf("[+] ptrace call ret status is %d\n", stat);
    while (stat != 0xb7f){
        if (ptrace_continue(pid) == -1){
            printf("[-] ptrace call error");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }

    // 获取远程进程的寄存器值，方便获取返回值
    if (ptrace_getregs(pid, regs) == -1){
        printf("[-] After call getregs error");
        return -1;
    }

#elif defined(__x86_64__) // ？？
    int num_param_registers = 6;
    // x64处理器，函数传递参数，将整数和指针参数前6个参数从左到右保存在寄存器rdi,rsi,rdx,rcx,r8和r9
    // 更多的参数则按照从右到左的顺序依次压入堆栈。
    if (num_params > 0)
        regs->rdi = parameters[0];
    if (num_params > 1)
        regs->rsi = parameters[1];
    if (num_params > 2)
        regs->rdx = parameters[2];
    if (num_params > 3)
        regs->rcx = parameters[3];
    if (num_params > 4)
        regs->r8 = parameters[4];
    if (num_params > 5)
        regs->r9 = parameters[5];

    if (num_param_registers < num_params){
        regs->esp -= (num_params - num_param_registers) * sizeof(uintptr_t); // 分配栈空间，栈的方向是从高地址到低地址
        if (0 != ptrace_writedata(pid, (uint8_t *)regs->esp, (uint8_t *)&parameters[num_param_registers], (num_params - num_param_registers) * sizeof(uintptr_t))){
            return -1;
        }
    }

    uintptr_t tmp_addr = 0x0;
    regs->esp -= sizeof(uintptr_t);
    if (0 != ptrace_writedata(pid, (uint8_t *)regs->esp, (uint8_t *)&tmp_addr, sizeof(tmp_addr))){
        return -1;
    }

    //设置eip寄存器为需要调用的函数地址
    regs->eip = ExecuteAddr;

    // 开始执行
    if (-1 == ptrace_setregs(pid, regs) || -1 == ptrace_continue(pid)){
        printf("[-] ptrace set regs or continue error, pid:%d", pid);
        return -1;
    }

    int stat = 0;
    // 对于使用ptrace_cont运行的子进程，它会在3种情况下进入暂停状态：①下一次系统调用；②子进程退出；③子进程的执行发生错误。
    // 参数WUNTRACED表示当进程进入暂停状态后，立即返回
    waitpid(pid, &stat, WUNTRACED);

    // 判断是否成功执行函数
    printf("ptrace call ret status is %lX\n", stat);
    while (stat != 0xb7f){
        if (ptrace_continue(pid) == -1){
            printf("[-] ptrace call error");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }

#elif defined(__arm__) || defined(__aarch64__) // 真机
#if defined(__arm__) // 32位真机
    int num_param_registers = 4;
#elif defined(__aarch64__) // 64位真机
    int num_param_registers = 8;
#endif
    int i = 0;
    // ARM处理器，函数传递参数，将前四个参数放到r0-r3，剩下的参数压入栈中
    for (i = 0 ; i < num_params && i < num_param_registers ; i++) {
        regs->uregs[i] = parameters[i];
    }

    if (i < num_params) {
        regs->ARM_sp -= (num_params - i) * sizeof(uintptr_t); // 分配栈空间，栈的方向是从高地址到低地址
        if (ptrace_writedata(pid, (uint8_t *) (regs->ARM_sp), (uint8_t *) &parameters[i], (num_params - i) * sizeof(uintptr_t)) == -1) {
            return -1;
        }
    }

    regs->ARM_pc = ExecuteAddr; //设置ARM_pc寄存器为需要调用的函数地址
    // 与BX跳转指令类似，判断跳转的地址位[0]是否为1，如果为1，则将CPST寄存器的标志T置位，解释为Thumb代码
    // 若为0，则将CPSR寄存器的标志T复位，解释为ARM代码
    if (regs->ARM_pc & 1) {
        /* thumb */
        regs->ARM_pc &= (~1u);
        regs->ARM_cpsr |= CPSR_T_MASK;
    } else {
        /* arm */
        regs->ARM_cpsr &= ~CPSR_T_MASK;
    }

    regs->ARM_lr = 0;

    // Android 7.0以上修正lr为libc.so的起始地址 getprop获取ro.build.version.sdk
    uintptr_t lr_val = 0;
    char sdk_ver[32];
    memset(sdk_ver, 0, sizeof(sdk_ver));
    __system_property_get("ro.build.version.sdk", sdk_ver);
    //    printf("ro.build.version.sdk: %s", sdk_ver);
    if (atoi(sdk_ver) <= 23) {
        lr_val = 0;
    } else { // Android 7.0
        static uintptr_t start_ptr = 0;
        if (start_ptr == 0) {
            start_ptr = (uintptr_t) get_module_base_addr(pid, process_libs.libc_path);
        }
        lr_val = start_ptr;
    }
    regs->ARM_lr = lr_val;

    if (ptrace_setregs(pid, regs) == -1 || ptrace_continue(pid) == -1) {
        printf("[-] ptrace set regs or continue error, pid:%d\n", pid);
        return -1;
    }

    int stat = 0;
    // 对于使用ptrace_cont运行的子进程，它会在3种情况下进入暂停状态：①下一次系统调用；②子进程退出；③子进程的执行发生错误。
    // 参数WUNTRACED表示当进程进入暂停状态后，立即返回
    // 将ARM_lr（存放返回地址）设置为0，会导致子进程执行发生错误，则子进程进入暂停状态
    waitpid(pid, &stat, WUNTRACED);

    // 判断是否成功执行函数
    //printf("[+] ptrace call ret status is %d\n", stat);
    while ((stat & 0xFF) != 0x7f) {
        if (ptrace_continue(pid) == -1) {
            printf("[-] ptrace call error\n");
            return -1;
        }
        waitpid(pid, &stat, WUNTRACED);
    }

    // 获取远程进程的寄存器值，方便获取返回值
    if (ptrace_getregs(pid, regs) == -1) {
        printf("[-] After call getregs error\n");
        return -1;
    }

#else // 设备不符合注入器构架
    printf("[-] Not supported Environment %s\n", __FUNCTION__);
#endif
    return 0;
}

#pragma clang diagnostic pop