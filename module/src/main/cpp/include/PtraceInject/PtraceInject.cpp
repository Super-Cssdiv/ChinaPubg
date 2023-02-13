#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include <MapTools.h>
#include <dlfcn.h>
#include <string>
#include <sys/stat.h>
#include "inject.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
#define MEMORY_SIZE 1024

using namespace std;

struct hide_struct {
    procmaps_struct *original;
};

bool equals(const char *str1, const char *str2) {
    if (str1 == nullptr && str2 == nullptr) {
        return true;
    } else {
        if (str1 != nullptr && str2 != nullptr) {
            return strcmp(str1, str2) == 0;
        } else {
            return false;
        }
    }
}

//将文件1复制到文件2,如果文件2不存在,则创建文件2
void copyFile(const string& src, const string& dst) {
    FILE *in = fopen(src.c_str( ), "rb");
    if (in == nullptr) {
        printf("copyFile: open read %s failed\n", src.c_str( ));
        return;
    }
    FILE *out = fopen(dst.c_str( ), "wb");
    if (out == nullptr) {
        printf("copyFile: open write %s failed\n", dst.c_str( ));
        fclose(in);
        return;
    }
    char buf[1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        fwrite(buf, 1, n, out);
    }
    fclose(in);
    fclose(out);
}

string findAppPath(const char* package,const char *path) {
    DIR *dir = opendir(path);
    if (dir == nullptr) {
        return "";
    }
    std::string newDir = path;
    char end = *(--newDir.end());  // 字符串最后一个字符
    if (end != '/') {
        newDir = newDir + "/";
    }
    // 调用readdir( b)函数读取目录a下所有文件（包括目录）
    struct dirent *ptr;
    while ((ptr = readdir(dir)) != nullptr) {
        struct stat buff{ };
        if (std::strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
            // 目录(4)
        } else if (ptr->d_type == 4) {
            if(newDir.find(package) != std::string::npos){
                return newDir;
            }else{
                string result = findAppPath(package,(newDir + ptr->d_name + "/").c_str());
                if(result.length() > 0){
                    return result;
                }
            }
        }
    }
    return "";
}


static int get_prot(const procmaps_struct *procstruct) {
    int prot = 0;
    if (procstruct->is_r) {
        prot |= PROT_READ;
    }
    if (procstruct->is_w) {
        prot |= PROT_WRITE;
    }
    if (procstruct->is_x) {
        prot |= PROT_EXEC;
    }
    return prot;
}

bool hideLibraryMemory(pid_t pid, const char *lib, InjectTools *injectTools, void *memoryAddr) {
    procmaps_iterator *maps = pmparser_parse(pid);
    if (maps == nullptr) {
        return false;
    }
    //存储指定so的内存信息
    hide_struct *data = nullptr;
    //指定so的内存区域数量
    size_t dataCount = 0;
    //遍历内存maps
    procmaps_struct *tmpMaps;
    while ((tmpMaps = pmparser_next(maps)) != nullptr) {
        //判断是否是指定so的内存区域
        if (!equals(lib, tmpMaps->pathname)) {
            continue;
        }
        //printf("%p-%p %s %ld %s\n", tmpMaps->addr_start, tmpMaps->addr_end, tmpMaps->perm, tmpMaps->offset, tmpMaps->pathname);
        //判断内存是否为空,不为空则重新调整内存大小
        if (data) {
            data = (hide_struct *) realloc(data, sizeof(hide_struct) * (dataCount + 1));
        } else {
            //申请内存
            data = (hide_struct *) malloc(sizeof(hide_struct));
        }
        //记录maps内存区域信息
        data[dataCount].original = tmpMaps;
        //计次+1
        dataCount += 1;
    }
    printf("[+] 隐藏动态库开始\n");
    for (int i = 0 ; i < dataCount ; ++i) {
        auto start = (void*) data[i].original->addr_start;
        auto end = (void*)data[i].original->addr_end;
        auto length = (uintptr_t) end - (uintptr_t) start;
        //内存区域权限
        int prot = get_prot(data[i].original);
        //申请一片内存
        void *backupAddr = injectTools->callMmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (backupAddr == nullptr) {
            printf("[-] 申请内存失败\n");
            return false;
        }
        //原内存区域:是否有读属性,没有给他加上
        if (!data[i].original->is_r) {
            if ((uintptr_t) injectTools->callMprotect(start, length, PROT_READ) == -1) {
                printf("[-] 内存属性修改失败\n");
                return false;
            }
        }
        //将原内存区域复制到新内存区域
        injectTools->callMemcpy(backupAddr, start, length);
        //将原内存区域卸载掉
        if ((uintptr_t) injectTools->callMunmap(start, length) == -1) {
            printf("[-] 卸载内存失败\n");
            return false;
        }
        const char *str = "jit-cache";
        injectTools->writeRemoteMemory(memoryAddr, (void *) str, MEMORY_SIZE);
        uintptr_t parameters[3];
        parameters[0] = __NR_memfd_create;
        parameters[1] = (uintptr_t) memoryAddr;
        parameters[2] = MFD_CLOEXEC | MFD_ALLOW_SEALING;
        auto fd = (uintptr_t)injectTools->callSyscall(parameters, 3);
        //修改fd的文件大小为指定大小
        injectTools->callFtruncate(fd, length);
        //将内存写入到fd文件中
        injectTools->callWrite(fd, (void *)data[i].original->addr_start, length);
        //在原内存区域上新建一片内存,覆盖掉
        injectTools->callMmap(start, length, prot, MAP_PRIVATE, fd, 0);
        //关闭fd文件句柄
        injectTools->callClose(fd);

        //给原内存区域加上写权限
        injectTools->callMprotect(start, length, prot | PROT_WRITE);
        //拷贝新内存区域到原内存区域
        injectTools->callMemcpy(start, backupAddr, length);
        if (!data[i].original->is_w) {
            //还原原内存区域权限
            injectTools->callMprotect(start, length, prot);
        }
        //卸载新内存区域
        injectTools->callMunmap(backupAddr, length);

        printf("[+] 隐藏内存段 %p-%p %s 成功\n", data[i].original->addr_start, data[i].original->addr_end, data[i].original->perm);
    }
    //释放内存
    if (data) {
        free(data);
    }
    //释放maps数据
    pmparser_free(maps);

    return true;
}

int injectLibraryToRemote(pid_t pid, const char *lib, bool hide, const char *functionName, void *regs) {
    InjectTools injectTools = InjectTools(pid);
    //开始注入
    injectTools.injectStart();
    //在远程进程中申请一片内存
    void *memoryAddr = injectTools.callMmap(nullptr, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    //将lib的路径写入到远程进程的内存中
    injectTools.writeRemoteMemory(memoryAddr, (void *) lib, MEMORY_SIZE);
    //调用dlopen
    void *dlopenHandle = injectTools.callDlOpen(memoryAddr, RTLD_NOW | RTLD_GLOBAL);
    //判断是否成功
    if (dlopenHandle == nullptr) {
        printf("[-] 注入动态库 %s 失败\n", lib);
        return 0;
    }
    //隐藏动态库 匿名内存
    if (hide) {
        if (!hideLibraryMemory(pid, lib, &injectTools, memoryAddr)) {
            printf("[-] 隐藏动态库失败\n");
            return -1;
        }
    }
    //将函数名写入到远程进程的内存中
    injectTools.writeRemoteMemory(memoryAddr, (void *) functionName, MEMORY_SIZE);
    //调用dlsym查找函数
    void *functionAddr = injectTools.findFunction(dlopenHandle, memoryAddr);
    if (functionAddr == nullptr) {
        printf("[-] 查找函数函数失败\n");
        return -2;
    }
    if (regs != nullptr) {
        //将参数写入到远程进程的内存中
        injectTools.writeRemoteMemory(memoryAddr, regs, MEMORY_SIZE);
        //自定义函数的参数
        uintptr_t parameters[1];
        parameters[0] = (uintptr_t) memoryAddr;
        injectTools.callCustomFunction(functionAddr, &parameters, 1);
    } else {
        injectTools.callCustomFunction(functionAddr, nullptr, 0);
    }
    //释放内存
    injectTools.callMunmap(memoryAddr, MEMORY_SIZE);
    //调用dlclose关闭dlopen的handle
    injectTools.callDlClose(dlopenHandle);
    //注入结束
    injectTools.injectEnd( );

    return 1;
}

void setSoPath(){
    // 系统lib路径
    procmaps_iterator *maps = pmparser_parse(-1);
    procmaps_struct *tmpMaps;
    while ((tmpMaps = pmparser_next(maps)) != nullptr) {
        if (strstr(tmpMaps->pathname, "libc.so") != nullptr) {
            setLibcPath(tmpMaps->pathname);
        }
        if (strstr(tmpMaps->pathname, "linker") != nullptr && strstr(tmpMaps->pathname, "linker_alloc") == nullptr) {
            setLinkerPath(tmpMaps->pathname);
        }
        if (strstr(tmpMaps->pathname, "libdl.so") != nullptr) {
            setLibdlPath(tmpMaps->pathname);
        }
    }
    //释放maps数据
    pmparser_free(maps);
}


std::string getDeviceNumber() {
    char result[1024] = {0};
    FILE * fp = fopen("/sys/devices/soc0/serial_number", "r");
    if (fp == nullptr) {
        return "";
    }
    if (fgets(result, 1024, fp) != nullptr) {
        int resultLen = strlen(result);
        while (result[resultLen - 1] == '\n' || result[resultLen - 1] == '\r') {
            result[resultLen - 1] = '\0';
            resultLen--;
        }
    }
    fclose(fp);
    return result;
}

int main(int argc, char *argv[]) {
    //注入so
    const char *libPath = argv[1];
    const char *pkgName = argv[2];
    setSoPath();
    printf("[+] 注入动态库路径: %s\n", libPath);
    string copyLibPath =  findAppPath(pkgName,"/data/app/") + "lib/arm64/libPeace.so";
    //移动文件
    copyFile(libPath, copyLibPath);
    //赋值777权限
    system(("chmod 775 " + copyLibPath).c_str());

    system("settings put global block_untrusted_touches 0" );

    printf("[+] 等待进程启动\n");
    pid_t pid = -1;
    do {
        pid = get_pid_by_name("com.tencent.tmgp.pubgmhd");
    } while(pid == -1);
    //延迟一段时间
    usleep(2.5 * 1000000);

    printf("[+] 进程已启动:%d,等待注入\n", pid);
    //等待注入时机
    printf("[+] 开始注入动态库\n");
    const char *test = getDeviceNumber().c_str();
    int result = injectLibraryToRemote(pid, copyLibPath.c_str(), true, "initLib", (void *)test);
    //注入完成
    printf("[+] 注入结果:%d\n", result);
    //删除文件
    system(("rm -rf " + copyLibPath).c_str());
    return result;
}
#pragma clang diagnostic pop
#pragma clang diagnostic pop