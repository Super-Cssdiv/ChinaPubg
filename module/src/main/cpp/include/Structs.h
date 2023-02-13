#pragma once

#include <string>
#include <string.h>
#include <android/native_activity.h>
#include <android/configuration.h>
#include <imgui.h>
#include <MemoryTools.h>

using namespace std;

#define MaxPlayerCount 50
#define MaxItemsCount 30
#define MaxVehicleCount 30
#define MaxGrenadeCount 30
#define MaxAirBoxCount 30
#define MaxActorCount 1024

#define KEY_VOLUMEDOWN 114

struct HookState {
    bool track;//追踪
    bool recoil;//枪械无后
    bool Rate;// 射速
    bool Gathering;

    HookState() {
        this->track = false;
        this->recoil = false;
        this->Rate = false;
        this->Gathering = false;
    }
};

struct HashMap {
    uintptr_t addr;
    size_t size;
};

struct Vec4 {
    float X, Y, Z, W;
};

struct Vector2A {
    float X;
    float Y;

    Vector2A() {
        this->X = 0;
        this->Y = 0;
    }

    Vector2A(float x, float y) {
        this->X = x;
        this->Y = y;
    }
};

static struct Color {
    ImVec4 Red = {255 / 255.f, 0 / 255.f, 0 / 255.f, 255 / 255.f};
    ImVec4 Red_ = {255 / 255.f, 0 / 255.f, 0 / 255.f, 50 / 255.f};
    ImVec4 Green = {0 / 255.f, 255 / 255.f, 0 / 255.f, 255 / 255.f};
    ImVec4 Green_ = {0 / 255.f, 255 / 255.f, 0 / 255.f, 117 / 255.f};
    ImVec4 White = {1.0, 1.0, 1.0, 1.0};
    ImVec4 White_ = {255 / 255.f, 255 / 255.f, 255 / 255.f, 180.f / 255.f};
    ImVec4 Black = {0 / 255.f, 0 / 255.f, 0 / 255.f, 255.f / 255.f};
    ImVec4 Black_ = {0 / 255.f, 0 / 255.f, 0 / 255.f, 117.f / 255.f};
    ImVec4 Yellow = {255 / 255.f, 255 / 255.f, 0, 255 / 255.f};
} Color;


//屏幕坐标结构
struct Rect {
    float x;
    float y;
    float w;
    float h;
};

struct Vector3A {
    float X;
    float Y;
    float Z;

    Vector3A() {
        this->X = 0;
        this->Y = 0;
        this->Z = 0;
    }

    Vector3A(float x, float y, float z) {
        this->X = x;
        this->Y = y;
        this->Z = z;
    }

};

struct Vector4A {
    float x;
    float y;
    float z;
    float w;

    Vector4A() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
        this->w = 0;
    }


    Vector4A(float x, float y, float h, float w) {
        this->x = x;
        this->y = y;
        this->z = h;
        this->w = w;
    }
};


struct BoneData {
    Vector3A Head;
    Vector3A vNeck;
    Vector3A Chest;
    Vector3A Pelvis;
    Vector3A Left_Shoulder;
    Vector3A Right_Shoulder;
    Vector3A Left_Elbow;
    Vector3A Right_Elbow;
    Vector3A Left_Wrist;
    Vector3A Right_Wrist;
    Vector3A Left_Thigh;
    Vector3A Right_Thigh;
    Vector3A Left_Knee;
    Vector3A Right_Knee;
    Vector3A Left_Ankle;
    Vector3A Right_Ankle;
};


struct android_poll_source {
    int32_t id;
    struct android_app *app;

    void (*process)(struct android_app *app, struct android_poll_source *source);
};


struct android_app {
    // The application can place a pointer to its own state object
    // here if it likes.
    void *userData;

    // Fill this in with the function to process main app commands (APP_CMD_*)
    void (*onAppCmd)(struct android_app *app, int32_t cmd);

    // Fill this in with the function to process input events.  At this point
    // the event has already been pre-dispatched, and it will be finished upon
    // return.  Return 1 if you have handled the event, 0 for any default
    // dispatching.
    int32_t (*onInputEvent)(struct android_app *app, AInputEvent *event);

    // The ANativeActivity object instance that this app is running in.
    ANativeActivity *activity;

    // The current configuration the app is running in.
    AConfiguration *config;

    // This is the last instance's saved state, as provided at creation time.
    // It is NULL if there was no state.  You can use this as you need; the
    // memory will remain around until you call android_app_exec_cmd() for
    // APP_CMD_RESUME, at which point it will be freed and savedState set to NULL.
    // These variables should only be changed when processing a APP_CMD_SAVE_STATE,
    // at which point they will be initialized to NULL and you can malloc your
    // state and place the information here.  In that case the memory will be
    // freed for you later.
    void *savedState;
    size_t savedStateSize;

    // The ALooper associated with the app's thread.
    ALooper *looper;

    // When non-NULL, this is the input queue from which the app will
    // receive user input events.
    AInputQueue *inputQueue;

    // When non-NULL, this is the window surface that the app can draw in.
    ANativeWindow *window;

    // Current content rectangle of the window; this is the area where the
    // window's content should be placed to be seen by the user.
    ARect contentRect;

    // Current state of the app's activity.  May be either APP_CMD_START,
    // APP_CMD_RESUME, APP_CMD_PAUSE, or APP_CMD_STOP; see below.
    int activityState;

    // This is non-zero when the application's NativeActivity is being
    // destroyed and waiting for the app thread to complete.
    int destroyRequested;

    // -------------------------------------------------
    // Below are "private" implementation of the glue code.

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int msgread;
    int msgwrite;

    pthread_t thread;

    struct android_poll_source cmdPollSource;
    struct android_poll_source inputPollSource;

    int running;
    int stateSaved;
    int destroyed;
    int redrawNeeded;
    AInputQueue *pendingInputQueue;
    ANativeWindow *pendingWindow;
    ARect pendingContentRect;
};

struct ItemData {
    float Distance;
    Vector2A Location;
    int ItemNum;
};

struct TextureInfo {
    ImTextureID textureId;
    int width;
    int height;
};

struct DisplayMetrics {
    int widthPixels;
    int heightPixels;
    float density;
};

struct WeaponInfo {
    int id;
    int CurrentBullet;
    int TotalBullet;
};

struct PlayerData {
    bool isAI;
    bool isRat;
    int TeamID;
    float Health;
    float Angle;
    int State;
    bool isVisibility;
    float Distance;
    char PlayerName[64];
    float HeadSize;
    WeaponInfo Weapon{0,0,0};
    BoneData mBoneData;
    Vector4A Location;
    Vector2A RadarLocation;
    Vector2A RotationAngle;
};


struct FMatrix {
    float M[4][4];
};

struct Quat {
    float X;
    float Y;
    float Z;
    float W;
};


struct Update {
    bool isRat;
    bool isAI;
    int TeamID;
    uintptr_t actorBases;
    char PlayerName[64];
    uintptr_t plater_mesh_;
    uintptr_t RootCompont;//坐标指针
    //物品
    char ItemName[128];//类名
    char ClassPath[128]; //类
};




struct Rotator {
    float Y;//P
    float X;//Y
    float Roll;
};

struct Tracking {
    Rotator aim_angle;
    Vector3A loc;
};

typedef struct {
    int type;
    int isTask;
    void *args;
    int isReturn;
    void *ret;
    pthread_t thread;
    pthread_mutex_t mutex;
} thread_syscall_t;

static uint16_t get_utf16(char *str, int *index)
{
    int i = *index;
    if ((str[i] & 0x80) == 0)
    {
        *index = i + 1;
        return str[i];
    }
    else if ((str[i] & 0x20) == 0)
    {
        *index = i + 2;
        return ((str[i] & 0x1f) << 6) | (str[i + 1] & 0x003f);
    }
    else if ((str[i] & 0x10) == 0)
    {
        *index = i + 3;
        return ((str[i] & 0xf) << 12) | ((str[i + 1] & 0x3f) << 6) | (str[i + 2] & 0x003f);
    }
    return 0;
}

static void set_utf16(char *str, int *index, uint16_t u16c)
{
    int i = *index;
    if (u16c >= 0x800)
    {
        *index = i + 3;
        str[i] = (u16c >> 12) | 0xe0;
        str[i + 1] = ((u16c >> 6) & 0x3f) | 0x80;
        str[i + 2] = (u16c & 0x3f) | 0x80;
    }
    else if (u16c >= 0x80)
    {
        *index = i + 2;
        str[i] = ((u16c >> 6) & 0x1f) | 0xc0;
        str[i + 1] = (u16c & 0x3f) | 0x80;
    }
    else
    {
        *index = i + 1;
        str[i] = (uint8_t)u16c;
    }
}

static int utf8_to_utf16(char *u8str, uint16_t *u16str)
{
    int i, j;
    for (i = 0, j = 0; (u16str[j] = get_utf16(u8str, &i)) != 0; j++);
    return j;
}

static int utf16_to_utf8(uint16_t *u16str, char *u8str)
{
    int i, j;
    i = 0, j = 0;
    do
    {
        set_utf16(u8str, &i, u16str[j]);
    }
    while (u16str[j++] != 0);
    return i;
}

template<class T>
struct TArray
{
    friend struct FString;

public:
    inline TArray()
    {
        Data = nullptr;
        Count = Max = 0;
    };

    inline int Num() const
    {
        return Count;
    };

    inline T& operator[](int i)
    {
        return Data[i];
    };

    inline const T& operator[](int i) const
    {
        return Data[i];
    };

    inline bool IsValidIndex(int i) const
    {
        return i < Num();
    }

private:
    T* Data = nullptr;
    int32_t Count;
    int32_t Max;
};

struct FString : private TArray<char>
{
    FString()
    {
    }

    bool IsValid() const
    {
        return IsPtrValid(Data);
    }

    void ToString(char *ret) const
    {
        if (IsValid())
            utf16_to_utf8((uint16_t *)Data, ret);
    }
};
