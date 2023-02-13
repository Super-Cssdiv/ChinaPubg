#include <cstring>
#include <TencentSafe.h>
#include <Logger.h>
#include <elf_util.h>
#include <SubstrateHook.h>
#include <dlfcn.h>

static int (*tp2_sdk_init_ex)(int app_id, const char* app_key) = nullptr;
static uintptr_t (*tp2_sdk_ioctl)(int request, const char *param_str) = nullptr;
static int (*tss_sdk_ioctl)(int request, const void *param, char *buf, size_t buf_size, size_t *used_buf_len) = nullptr;
static int (*tp2_setuserinfo)(int account_type, int world_id, const char *open_id, const char *role_id) = nullptr;

void dlopen_process(const char *name, void *handle) {
    if(name == nullptr || handle == nullptr){
        return;
    }
    if (strstr(name, "libtersafe.so")) {
        *(void**) &tp2_sdk_init_ex = dlsym(handle,"tp2_sdk_init_ex");
        *(void**) &tp2_setuserinfo = dlsym(handle,"tp2_setuserinfo");
        *(void**) &tp2_sdk_ioctl = dlsym(handle,"tp2_sdk_ioctl");
        *(void**) &tss_sdk_ioctl = dlsym(handle,"tss_sdk_ioctl");
        tp2_sdk_init_ex(9000, "d5ab8dc7ef67ca92e41d730982c5c602");
        if (tp2_sdk_ioctl(TssSDKCmd_CommQuery,"CloseDevInfoCollect:0xfff") == 0) {
            //LOGD("[info] CloseDevInfoCollect Success");
        }
        if (tp2_sdk_ioctl(TssSDKCmd_CommQuery,"CloseUserTagScan") == 0) {
            //LOGD("[info] CloseUserTagScan Success");
        }
        if (tp2_sdk_ioctl(TssSDKCmd_CommQuery,"CloseAPKCollect") == 0) {
            //LOGD("[info] CloseAPKCollect Success");
        }
        if (tp2_sdk_ioctl(TssSDKCmd_CommQuery,"CloseAntiProfiler") == 0) {
            //LOGD("[info] CloseAntiProfiler Success");
        }
        char mtpBuff[BUFSIZ] = {0};
        if (tss_sdk_ioctl(TssSDKCmd_CommQuery,"CloseDevInfoCollect:0xfff", mtpBuff, 0 ,nullptr) == 0) {
            //LOGD("[info] CloseDevInfoCollect Success");
        }
    }
}

HOOK_DEF(void*, __loader_dlopen, const char *filename, int flags, const void *caller_addr) {
    void *handle = orig___loader_dlopen(filename, flags, caller_addr);
    dlopen_process(filename, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V24, const char *name, int flags, const void *extinfo, void *caller_addr) {
    void *handle = orig_do_dlopen_V24(name, flags, extinfo, caller_addr);
    dlopen_process(name, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V19, const char *name, int flags, const void *extinfo) {
    void *handle = orig_do_dlopen_V19(name, flags, extinfo);
    dlopen_process(name, handle);
    return handle;
}

void TencentSafe::init() {
    int api_level = android_get_device_api_level();
    //LOGD("[info] api level: %d", api_level);
    const char *linker = api_level >= 29 ? "/apex/com.android.runtime/bin/linker64" : "/system/bin/linker64";
    if (api_level >= 30) {
        void *addr = reinterpret_cast<void *>(SandHook::Elf::ElfImg(linker).GetSymAddress("__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv"));
        if (addr) {
            //LOGD("[info] do_dlopen at: %p", addr);
            MSHookFunction(addr, (void *) new_do_dlopen_V24,(void **) &orig_do_dlopen_V24);
        }
    } else if (api_level >= 26) {
        void *libdl_handle = dlopen("libdl.so", RTLD_LAZY);
        void *addr = dlsym(libdl_handle, "__loader_dlopen");
        //LOGD("[info] __loader_dlopen at: %p", addr);
        MSHookFunction(addr, (void *) new___loader_dlopen,(void **) &orig___loader_dlopen);
    } else if (api_level >= 24) {
        void *addr = reinterpret_cast<void *>(SandHook::Elf::ElfImg(linker).GetSymAddress("__dl__Z9do_dlopenPKciPK17android_dlextinfoPv"));
        if (addr) {
            //LOGD("[info] do_dlopen at: %p", addr);
            MSHookFunction(addr, (void *) new_do_dlopen_V24,(void **) &orig_do_dlopen_V24);
        }
    } else {
        void *addr = reinterpret_cast<void *>(SandHook::Elf::ElfImg(linker).GetSymAddress("__dl__Z9do_dlopenPKciPK17android_dlextinfo"));
        if (addr) {
            //LOGD("[info] do_dlopen at: %p", addr);
            MSHookFunction(addr, (void *) new_do_dlopen_V19,(void **) &orig_do_dlopen_V19);
        }
    }
}