#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include <jni.h>
#include <sys/types.h>
#include <malloc.h>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <fcntl.h>
#include <Logger.h>
#include <cstring>
#include <sys/wait.h>
#include <sys/socket.h>
#include <MainLooper.h>
#include <hide_utils.h>
#include <dlfcn.h>
#include <MapTools.h>
#include <Structs.h>
#include "Menu.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "DanglingPointer"

thread_syscall_t *syscall_thread_ptr;

extern "C" __attribute__ ((visibility ("default"))) int initLib(const char *code) {
    MainLooper::GetInstance()->init(handle_message);
    pthread_t t;
    pthread_create(&t, nullptr, hook_read_thread, nullptr);
    return 1;
}

__attribute__((constructor)) void init_main() {
    syscall_thread_ptr = pthread_syscall_create();
    install_filter();
}



#pragma clang diagnostic pop
#pragma clang diagnostic pop