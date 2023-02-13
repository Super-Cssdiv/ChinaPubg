#include <MainLooper.h>
#include <unistd.h>
#include <pthread.h>

MainLooper *MainLooper::g_MainLooper = nullptr;


MainLooper *MainLooper::GetInstance() {
    if (!g_MainLooper) {
        g_MainLooper = new MainLooper();
    }
    return g_MainLooper;
}

MainLooper::MainLooper(){
    pthread_mutex_init(&looper_mutex_, nullptr);
}

MainLooper::~MainLooper() {
    if (mainlooper && readpipe != -1) {
        ALooper_removeFd(mainlooper, readpipe);
    }
    if (readpipe != -1) {
        close(readpipe);
    }
    if (writepipe != -1) {
        close(writepipe);
    }
    pthread_mutex_destroy(&looper_mutex_);
}

void MainLooper::init(ALooper_callbackFunc callback) {
    int msgpipe[2];
    pipe(msgpipe);
    readpipe = msgpipe[0];
    writepipe = msgpipe[1];
    mainlooper = ALooper_prepare(0);
    ALooper_addFd(mainlooper, readpipe, 1, ALOOPER_EVENT_INPUT, callback, nullptr);
}


void MainLooper::send(Message msg) {
    pthread_mutex_lock(&looper_mutex_);
    write(writepipe, &msg, sizeof(Message));
    pthread_mutex_unlock(&looper_mutex_);
}