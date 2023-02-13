#pragma once

#include <android/looper.h>
#include <android/input.h>

extern int versionCode;
extern thread_syscall_t *syscall_thread_ptr;
extern int uid;
static bool g_Initialized = false;

static bool isTestLogin = false;

//用于控制登录弹窗
static bool isLogin = false;
static time_t firstTime = 0;
static bool my_window_open = false;
static bool my_window_focused = false;
static bool isImageDown = false;
static char main_window_title[256] = {0};
static bool HasWindows;
static bool isSetWindowPos = false;
static bool showMemu = true;
static bool KernelState = false;
static bool initHeartbeat = false;
static bool WantTextInputLast = false;
static char *ptr;
static bool isShowDialog = false;
static bool initUpdateList = false;

thread_syscall_t *pthread_syscall_create();
void *hook_read_thread(void *);
int handle_message(int fd, __attribute__((unused)) int events, __attribute__((unused)) void *data);
void *thread_onDraw(void * args);
void hookTersafeSyscall();
void install_filter();
int lock(thread_syscall_t *syscall_thread);
int unlock(thread_syscall_t *syscall_thread);
void *call_task(thread_syscall_t *syscall_thread, void *args, int type);
