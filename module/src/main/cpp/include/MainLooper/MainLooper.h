#pragma once
#include <android/looper.h>
#include <string>

struct Message {
    int what;
    void *data;
};

class MainLooper {
public:
    static MainLooper *GetInstance();
    ~MainLooper();
    void init(ALooper_callbackFunc callback);
    void send(Message msg);

private:
    static MainLooper *g_MainLooper;
    MainLooper();
    ALooper* mainlooper;
    int readpipe;
    int writepipe;
    pthread_mutex_t looper_mutex_;
};