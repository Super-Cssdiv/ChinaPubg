#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#pragma ide diagnostic ignored "bugprone-integer-division"
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"

#include <cstdint>
#include <string>
#include <EGL/egl.h>
#include <Draw.h>
#include <linux/prctl.h>
#include "MapTools.h"
#include "GameUtils.h"
#include "MemoryTools.h"
#include "Structs.h"
#include "Engine.h"
#include "GameOffsets.h"
#include <sys/prctl.h>
#include <map>
#include <imgui_expand.h>

using namespace std;


//游戏函数原型
static uintptr_t (*GetPlayerCharacter)(void * WorldContextObject, int PlayerIndex) = nullptr;
static bool (*LineOfSightTo)(void *controller, void *actor, Vector3A ViewPoint,bool bAlternateChecks) = nullptr;
static Vector3A (*GetBoneLocation)(void *mesh, int BoneName, char Space) = nullptr;
static int (*GetBoneName)(void *mesh, int BoneIndex) = nullptr;
static void (*AddControllerYawInput)(void *myBase, float val) = nullptr;
static void (*AddControllerPitchInput)(void *myBase, float val) = nullptr;
static int (*GetNumBones)(void *mesh) = nullptr;

static bool isGetBase = false;
static uintptr_t uWorldAddress = 0;
static uintptr_t PersistentLevel = 0;
static uintptr_t NetDriver = 0;
static uintptr_t uMyAddress = 0;
//队伍自身
static int SelfTeamID = 0;
static int aimCount = 0;

//用于数据更新
static int actorsCount = 0;
static Update update[MaxActorCount + 5];
static Update update_data[MaxActorCount + 10];

static int updateActorsCount = 0;
static bool isRead = false;
static bool isSend = false;
static int BoneIndexs[65] = {0};

int PlayerCount = 0;
int VehicleCount = 0;
int ItemsCount = 0;
int AirBoxCount = 0;
int GrenadeCount = 0;

//开关控制
static bool isPeople = true;
static bool isItems = true;
static bool isVehicle = true;
static bool isMissiles = true;

//自瞄
static int bIsGunADS = 0;
static int myIsGunADS = 0;
float record_Max_shake = 0.3;

int my_aimedMode = 0;//自瞄模式
int bIsPressingFireBtn = 0;
static uintptr_t aimData = 0;
static uintptr_t uCurrentVehicle = 0;
static float ShakeValue = 0;//当前抖动值
static float Orientation = 1.0f;//当前抖动方向
static bool isMyProjSomoke = false;// 烟雾不瞄
static bool isHookAngle = false;
static float aimRange = 150.f;
extern int screenWidth, screenHeight;
extern int glWidth, glHeight;
bool openAccumulation;
int lockPart;
static Vector3A aimObjInfo;//NOLINT
static uintptr_t tmpGunBase = 0;//手持
static uintptr_t aimObjaddr = 0;
//颜色透明度
static float tm = 255 / 255.f;
static ImVec4 arr[] = {{144 / 255.f, 238 / 255.f, 144 / 255.f, tm},
                       {135 / 255.f, 206 / 255.f, 255 / 255.f, tm},
                       {255 / 255.f, 0 / 255.f,   0 / 255.f, tm},
                       {0 / 255.f,   255 / 255.f, 0 / 255.f, tm},
                       {0 / 255.f,   255 / 255.f, 127 / 255.f, tm},
                       {255 / 255.f, 182 / 255.f, 193 / 255.f, tm},
                       {218 / 255.f, 112 / 255.f, 214 / 255.f, tm},
                       {248 / 255.f, 248 / 255.f, 255 / 255.f, tm},
                       {0 / 255.f,   255 / 255.f, 255 / 255.f, tm},
                       {255 / 255.f, 165 / 255.f, 0 / 255.f,   tm},
                       {153 / 255.f, 204 / 255.f, 255 / 255.f, tm},
                       {204 / 255.f, 255 / 255.f, 153 / 255.f, tm},
                       {255 / 255.f, 255 / 255.f, 153 / 255.f, tm},
                       {255 / 255.f, 153 / 255.f, 153 / 255.f, tm},
                       {153 / 255.f, 153 / 255.f, 204 / 255.f, tm},
                       {204 / 255.f, 204 / 255.f, 204 / 255.f, tm},
                       {102 / 255.f, 204 / 255.f, 153 / 255.f, tm},
                       {255 / 255.f, 102 / 255.f, 0 / 255.f,   tm},
                       {102 / 255.f, 204 / 255.f, 204 / 255.f, tm},
                       {153 / 255.f, 204 / 255.f, 255 / 255.f, tm}
};//NOLINT
static int length = sizeof(arr) / 20;//NOLINT


string GetFNameFromID(unsigned int index) {
    uintptr_t FNameEntryArr = getA(GnameAddress + ((index / 0x4000) * Offset::PointerSize));
    uintptr_t FNameEntry = getA(FNameEntryArr + ((index % 0x4000) * Offset::PointerSize));
    uintptr_t address = FNameEntry + Offset::FNameEntryToNameString;
    if (!isSafeAddress(address, 4))
        return "error";
    return reinterpret_cast<char *>(address);
}

uintptr_t getClass(uintptr_t object) {
    return getA(object + Offset::UObjectToClassPrivate);
}

int getNameID(uintptr_t object) {
    return getI(object + Offset::UObjectToInternalIndex);
}

string getName(uintptr_t object) {
    return GetFNameFromID(getNameID(object));
}

/* 计算3D坐标距离 */
float getDistance(float zx, float zy, float zz, float dx, float dy, float dz) {
    float x, y, z;
    x = zx - dx;
    y = zy - dy;
    z = zz - dz;
    return (float) (sqrt(x * x + y * y + z * z));
}

//获取屏幕中心距离
float get2dDistance(float x, float y, float x1, float y1) {
    float xx1 = x - x1;
    float yy2 = y - y1;
    // 取平方根
    return sqrt(xx1 * xx1 + yy2 * yy2);
}

static bool isContain(string str, const char *check) {
    if (check == nullptr || str.empty()) return false;
    return str.find(check) != string::npos;
}

// 计算旋转坐标
Vector2A rotateCoord(float angle, float objectRadar_x, float objectRadar_y) {
    float s = sin(angle * M_PI / 180);
    float c = cos(angle * M_PI / 180);
    return {objectRadar_x * c + objectRadar_y * s,-objectRadar_x * s + objectRadar_y * c};
}

static float caculDis(Vector3A vPos1, Vector3A vPos2) {
    return sqrt(pow(vPos1.X - vPos2.X, 2) + pow(vPos1.Y - vPos2.Y, 2) + pow(vPos1.Z - vPos2.Z, 2)) /100.0f;
}

static Vector3A getBoneLoc(uintptr_t Mesh, int BoneIndex) {
    if (!isSafeAddress(Mesh, Offset::PointerSize))
        return {0, 0, 0};
    uintptr_t bone = getA(Mesh + Offset::BoneOffset);
    if (!isSafeAddress(bone, Offset::PointerSize))
        return {0, 0, 0};
    if (GetNumBones != nullptr && GetNumBones(reinterpret_cast<void *>(Mesh)) < 8) {
        return {0, 0, 0};
    }
    BoneIndex += 1;
    if (GetBoneLocation != nullptr && GetBoneName != nullptr) {
        if (Mesh <= 0x10000000 || Mesh % 4 != 0 || Mesh >= 0x10000000000) {
            return {0, 0, 0};
        }
        int Name = BoneIndexs[BoneIndex];
        if (Name <= 0) {
            Name = GetBoneName(reinterpret_cast<void *>(Mesh), BoneIndex);
            BoneIndexs[BoneIndex] = Name;
        }
        return GetBoneLocation(reinterpret_cast<void *>(Mesh), Name, 0);
    }
    return {0, 0, 0};
}

static bool isSafeAddr(Vector3A point) {
    if (isnan(point.X) || isinf(point.X) || isnan(point.Y) ||isinf(point.Y) || isnan(point.Z) ||isinf(point.Z) ) {
        return true;
    } else {
        return false;
    }
}

static bool isVisiblePoint(Vector3A point) {
    if (isSafeAddr(point)) return false;
    if (point.X == 0 && point.Y == 0 && point.Z == 0) return false;
    string playerControllerClassName = getName(playerController);
    string cameraManagerClassName = getName(playerCameraManagerBase);
    if (LineOfSightTo != nullptr && (isContain(playerControllerClassName, "PlayerController") || isContain(playerControllerClassName, "Liangzicontroller")) && isContain(cameraManagerClassName, "CameraManager")) {
        return LineOfSightTo(reinterpret_cast<void *>(playerController),reinterpret_cast<void *>(playerCameraManagerBase), point, false);
    }
    return false;
}

static bool isSafeAddr(float x, float y) {
    if(isnan(x) ||isinf(x) || isnan(y) ||isinf(y)){
        return false;
    } else {
        return true;
    }
}

bool worldToScreen(Vector3A *WorldLocationPtr, Vector4A *pOut) {
    FVector world = FVector(WorldLocationPtr->X, WorldLocationPtr->Y, WorldLocationPtr->Z + 100);
    FVector screen;
    bool isShow = CameraManager::WorldToScreen(world, &screen);

    FVector world2 = FVector(WorldLocationPtr->X, WorldLocationPtr->Y,WorldLocationPtr->Z + 285);
    FVector screen2;
    CameraManager::WorldToScreen(world2, &screen2);

    float h = screen.y - screen2.y;
    float w = h / 1.5f;
    float x = screen.x - h / 4 - w * 0.125f;

    pOut->x = x;
    pOut->y = screen.y;
    pOut->w = w;
    pOut->z = h;
    return isShow;
}

float worldToScreen(Vector3A *WorldLocationPtr, Vector2A *pOut) {
    FVector world = FVector(WorldLocationPtr->X, WorldLocationPtr->Y, WorldLocationPtr->Z);
    FVector screen;
    if (!CameraManager::WorldToScreen(world, &screen)){
        return -1;
    }
    pOut->X = screen.x - 10;
    pOut->Y = screen.y;
    Vector3A location;
    memoryRead(playerCameraManagerBase + Offset::dw_CameraCacheEntry + Offset::dw_MinimalViewInfo + Offset::dw_Location,&location, sizeof(location));
    return caculDis(*WorldLocationPtr, location);
}

bool getisDie(int state, float health) {
    return (state == 1048576 || state == 0 || state == 1048577 || state == 1048592 || state == 71894088 || health == 0);
}

static bool WorldToScreenPoint(Vector3A Enemy, Vector3A &Screen) {
    FVector world = FVector(Enemy.X, Enemy.Y, Enemy.Z);
    FVector screen;
    bool isShow = CameraManager::WorldToScreen(world, &screen);
    Screen.X = screen.x;
    Screen.Y = screen.y;
    isVisiblePoint(Enemy) ? Screen.Z = 1 : Screen.Z = 0;
    return isShow;
}

void DrawBone(uintptr_t mesh, BoneData *skeleton) {
    Vector3A vHead, vChest, vNeck, vPelvis, vShoulderL, vShoulderR, vElbowL, vElbowR, vWristL, vWristR, vLegL, vLegR, vKneeL, vKneeR, vAnkleL, vAnkleR;
    vHead = getBoneLoc(mesh, 6);
    vNeck = getBoneLoc(mesh, 5);
    vChest = getBoneLoc(mesh, 4);
    vPelvis = getBoneLoc(mesh, 1);
    vShoulderL = getBoneLoc(mesh, 11);
    vShoulderR = getBoneLoc(mesh, 32);
    vElbowL = getBoneLoc(mesh, 12);
    vElbowR = getBoneLoc(mesh, 33);
    vWristL = getBoneLoc(mesh, 63);
    vWristR = getBoneLoc(mesh, 62);
    vLegL = getBoneLoc(mesh, 52);
    vLegR = getBoneLoc(mesh, 56);
    vKneeL = getBoneLoc(mesh, 53);
    vKneeR = getBoneLoc(mesh, 57);
    vAnkleL = getBoneLoc(mesh, 54);
    vAnkleR = getBoneLoc(mesh, 58);
    vHead.X = vNeck.X;
    vHead.Y = vNeck.Y;
    vHead.Z = vNeck.Z + 7;
    WorldToScreenPoint(vHead, skeleton->Head);
    WorldToScreenPoint(vNeck, skeleton->vNeck);
    WorldToScreenPoint(vChest, skeleton->Chest);
    WorldToScreenPoint(vPelvis, skeleton->Pelvis);
    WorldToScreenPoint(vShoulderL, skeleton->Left_Shoulder);
    WorldToScreenPoint(vShoulderR, skeleton->Right_Shoulder);
    WorldToScreenPoint(vElbowL, skeleton->Left_Elbow);
    WorldToScreenPoint(vElbowR, skeleton->Right_Elbow);
    WorldToScreenPoint(vWristL, skeleton->Left_Wrist);
    WorldToScreenPoint(vWristR, skeleton->Right_Wrist);
    WorldToScreenPoint(vLegL, skeleton->Left_Thigh);
    WorldToScreenPoint(vLegR, skeleton->Right_Thigh);
    WorldToScreenPoint(vKneeL, skeleton->Left_Knee);
    WorldToScreenPoint(vKneeR, skeleton->Right_Knee);
    WorldToScreenPoint(vAnkleL, skeleton->Left_Ankle);
    WorldToScreenPoint(vAnkleR, skeleton->Right_Ankle);
}

static Vector3A aimBoneLocation(uintptr_t mesh) {
    BoneData boneData;
    boneData.Chest = getBoneLoc(mesh, 4);
    if (isVisiblePoint(boneData.Chest)) {
        return boneData.Chest;
    }
    boneData.Pelvis = getBoneLoc(mesh, 1);
    if (isVisiblePoint(boneData.Pelvis)) {
        return boneData.Pelvis;
    }
    boneData.Left_Shoulder = getBoneLoc(mesh, 11);
    if (isVisiblePoint(boneData.Left_Shoulder)) {
        return boneData.Left_Shoulder;
    }
    boneData.Right_Shoulder = getBoneLoc(mesh, 32);
    if (isVisiblePoint(boneData.Right_Shoulder)) {
        return boneData.Right_Shoulder;
    }
    boneData.Left_Elbow = getBoneLoc(mesh, 12);
    if (isVisiblePoint(boneData.Left_Elbow)) {
        return boneData.Left_Elbow;
    }
    boneData.Right_Elbow = getBoneLoc(mesh, 33);

    if (isVisiblePoint(boneData.Right_Elbow)) {
        return boneData.Right_Elbow;
    }
    boneData.Left_Wrist = getBoneLoc(mesh, 63);

    if (isVisiblePoint(boneData.Left_Wrist)) {
        return boneData.Left_Wrist;
    }
    boneData.Right_Wrist = getBoneLoc(mesh, 62);
    if (isVisiblePoint(boneData.Right_Wrist)) {
        return boneData.Right_Wrist;
    }
    boneData.Left_Thigh = getBoneLoc(mesh, 52);
    if (isVisiblePoint(boneData.Left_Thigh)) {
        return boneData.Left_Thigh;
    }
    // return {0, 0, 0};
    boneData.Right_Thigh = getBoneLoc(mesh, 56);
    if (isVisiblePoint(boneData.Right_Thigh)) {
        return boneData.Right_Thigh;
    }
    boneData.Left_Knee = getBoneLoc(mesh, 53);

    if (isVisiblePoint(boneData.Left_Knee)) {
        return boneData.Left_Knee;
    }
    boneData.Right_Knee = getBoneLoc(mesh, 57);
    if (isVisiblePoint(boneData.Right_Knee)) {
        return boneData.Right_Knee;
    }
    boneData.Left_Ankle = getBoneLoc(mesh, 54);

    if (isVisiblePoint(boneData.Left_Ankle)) {
        return boneData.Left_Ankle;
    }
    boneData.Right_Ankle = getBoneLoc(mesh, 58);
    if (isVisiblePoint(boneData.Right_Ankle)) {
        return boneData.Right_Ankle;
    }

    return {0, 0, 0};

}

static std::map<int, int> cacheWeapon;
static int increase[] = {
        101006//AUG
        ,101008//M762
        ,101003//SCAR-L
        ,101004//M416
        ,101004//M16A-4
        ,101009//Mk47
        ,101010//G36C
        ,101012//蜜獾
        ,101007//QBZ
        ,101001//AKM
        ,101005//Groza
        ,105001//M249
};

static bool getIncrease(int my_weapon) {
    return cacheWeapon.find(my_weapon) !=  cacheWeapon.end();
}

// 函数自瞄  对象地址 瞄准坐标
static void setAimLocation(uintptr_t ObjectPointer, Vector3A ObjInfo) {
    float aimAnticipation2 = aimAnticipation / 100; //预判
    //自己位置
    Vector3A myLocation;
    memoryRead(playerCameraManagerBase + Offset::dw_CameraCacheEntry +Offset::dw_MinimalViewInfo + Offset::dw_Location, &myLocation,sizeof(myLocation));
    Vector4A tmpLocation;
    if (!worldToScreen(&ObjInfo, &tmpLocation)) {
        return;
    }

    //my枪
    uintptr_t myGunBase = getA(getA(uMyAddress + Offset::weaponOffset) + Offset::gunOffset);

    //手持ID
    int my_weapon = getI(getA(uMyAddress + Offset::weaponOffset) + Offset::weaponIDOffset);

    //根据手持武器判断是否增加压枪
    float aimPress2;
    if(getIncrease(my_weapon)){
        aimPress2 = 0.45;
    }else{
        aimPress2 = 0;
    }

    //移动向量
    Vector3A moveLocation;
    memoryRead(ObjectPointer + Offset::VelocityOffset, &moveLocation,sizeof(moveLocation));

    //自身的移动向量
    Vector3A myMoveLocation;
    memoryRead(uMyAddress + Offset::VelocityOffset, &myMoveLocation,sizeof(myMoveLocation));

    moveLocation.X = moveLocation.X - myMoveLocation.X;
    moveLocation.Y = moveLocation.Y - myMoveLocation.Y;
    moveLocation.Z = moveLocation.Z - myMoveLocation.Z;

    // 距离
    double distance = sqrtf(powf(ObjInfo.X - myLocation.X, 2.0f) + powf(ObjInfo.Y - myLocation.Y, 2.0f) + powf(ObjInfo.Z - myLocation.Z, 2.0f));
    // 子弹速度
    double aimoSpeed = getF(myGunBase + Offset::bulletVelocityOffset) * 0.01f;
    // 枪口上抬
    double shangTai = (getF(getA(getA(uMyAddress + Offset::weaponOffset) + Offset::gunOffset) + Offset::resistanceOffset) + aimPress2) * aimPress;//* aimPress

    int myState = getI(uMyAddress + Offset::StateOffset);
    if (myState == 320) {
        //趴下腰射
        shangTai /= 4;
    } else if (myState == 280 || myState == 4368 || myState == 4369) {
        //蹲下腰射
        shangTai /= 3;
    } else if (myState == 1312 || myState == 5392 || myState == 5393) {
        //蹲下开镜
        shangTai /= 1.15;
    } else if (myState == 1344) {
        //趴下开镜
        shangTai /= 2.5;
    }

    //单发模式
    int fireTypeInt = getI(getA(uMyAddress + Offset::weaponOffset) + Offset::shootModeOffset);
    char fireType = (char) ((fireTypeInt >> 8) & 0xFF);
    if (fireType == 1) {
        shangTai = aimPress2 / 4;
    }

    int State = getI(ObjectPointer + Offset::StateOffset);
    if (State == 33554448) {
        shangTai = aimPress2 * 2;
    }

    if (State == 33554449) {
        shangTai = aimPress2 * 2.5f;
    }

    //到达耗时
    double flyTime = (distance / 100.0f) / aimoSpeed; //秒
    //下坠米
    double dropM = 0.0f;
    //飞行和修改耗时
    double useS = flyTime + aimAnticipation2;
    Vector3A yupanPoint;
    yupanPoint.X = ObjInfo.X + moveLocation.X * useS;
    yupanPoint.Y = ObjInfo.Y + moveLocation.Y * useS;
    yupanPoint.Z = ObjInfo.Z + moveLocation.Z * useS + dropM;

    double yuPanDistance = sqrtf(powf(yupanPoint.X - myLocation.X, 2.0f) +powf(yupanPoint.Y - myLocation.Y, 2.0f) +powf(yupanPoint.Z - myLocation.Z, 2.0f)) * 0.01f;
    double realFlyTime = yuPanDistance / aimoSpeed;
    double realUseS = realFlyTime + aimAnticipation2;
    yupanPoint.X = ObjInfo.X + moveLocation.X * realUseS;
    yupanPoint.Y = ObjInfo.Y + moveLocation.Y * realUseS;
    yupanPoint.Z = ObjInfo.Z + moveLocation.Z * realUseS + dropM;

    double zDistance = (myLocation.Z - yupanPoint.Z) * 0.01f;
    if (zDistance > 0.0f) {
        shangTai = shangTai + zDistance * 0.00147f;
    }
    double vax = 180.0f / PI;
    double vay = 180.0f / PI;
    double cx = yupanPoint.X - myLocation.X;
    double cy = yupanPoint.Y - myLocation.Y;
    double cz = yupanPoint.Z - myLocation.Z;
    double pfg = sqrtf((cx * cx) + (cy * cy));

    float max_shake2 = record_Max_shake;//= 0.5f + (float) (random() % 100) / 100.0f;//0.8- 0.99

    ShakeValue = ShakeValue + 0.05f * Orientation;
    if (fabs(ShakeValue) >= max_shake2) {
        Orientation = -Orientation;
    }

    double XGX, XGY;
    if (max_shake2 > 0) {
        XGX = atan2f(cy, cx) * vax + (fireType == 1 ? 0.0f : ShakeValue);
        XGY = atan2f(cz, pfg) * vay + (fireType == 1 ? 0.0f : ShakeValue);
    } else {
        XGX = atan2f(cy, cx) * vax;
        XGY = atan2f(cz, pfg) * vay;
    }
    if (XGX >= -180.0f && XGX < 0.0f) {
        XGX = XGX + 360.0f;
    }
    if (XGY < 0.0f && XGY >= -180.0f) {
        XGY = XGY + 360.0f;
    }
    XGY = XGY - shangTai;
    bool isOKYValue = XGY != NAN && XGX != NAN;

    if (aimbot && isOKYValue && (bIsPressingFireBtn || bIsGunADS)) {
        if (XGY >= 360.0f) {
            XGY = XGY - 360.0f;
        }
        if (XGY < 0.0f) {
            XGY = XGY + 360.0f;
        }
        float currentRotationY = getF(playerController + Offset::controlRotationOffset + 0);//P
        float currentRotationX = getF(playerController + Offset::controlRotationOffset + 4);//Y

        float needAddY = XGY - currentRotationY;//P
        float needAddX = XGX - currentRotationX;//Y

        //358 = -2
        //181 = -179
        if (needAddX > 180.0f) {
            needAddX = needAddX - 360.0f;
        }
        //-358 = 2
        //-181 = 179
        if (needAddX < -180.0f) {
            needAddX = 360.0f + needAddX;
        }

        //358 = -2
        //181 = -179
        if (needAddY > 180.0f) {
            needAddY = needAddY - 360.0f;
        }
        //-358 = 2
        //-181 = 179
        if (needAddY < -180.0f) {
            needAddY = 360.0f + needAddY;
        }

        needAddX = needAddX / aimSpeed + 0.01;//向右修正0.01
        needAddY = needAddY / aimSpeed * (aimSpeed == 1.0f ? 1.0f : 2.0f);

        XGX = currentRotationX + needAddX;
        XGY = currentRotationY + needAddY;
        //361
        if (XGX >= 360.0f) {
            XGX = XGX - 360.0f;
        }
        if (XGX < 0.0f) {
            XGX = XGX + 360.0f;
        }
        if (XGY >= 360.0f) {
            XGY = XGY - 360.0f;
        }
        if (XGY < 0.0f) {
            XGY = XGY + 360.0f;
        }
       // AddControllerInput(needAddY, needAddX);
    }

}

static FRotator ToRotator(const FVector &local, const FVector &target) {
    FVector rotation = local - target;
    float hyp = sqrt(rotation.x * rotation.x + rotation.y * rotation.y);
    FRotator newViewAngle;
    newViewAngle.Pitch = -atan(rotation.z / hyp) * (180.f / (float) 3.14159265358979323846);
    newViewAngle.Yaw = atan(rotation.y / rotation.x) * (180.f / (float) 3.14159265358979323846);
    newViewAngle.Roll = (float) 0.f;
    if (rotation.x >= 0.f)
        newViewAngle.Yaw += 180.0f;
    return newViewAngle;
}

// 追踪算法
static Tracking bulletTrack(Vector3A myLoc, bool isCusimg) {
    Vector3A yupanPoint;
    FRotator aim_angle;
    Tracking trackData;
    //自己位置
    Vector3A myLocation;
    if (isCusimg) {
        myLocation = myLoc;
    } else {
        memoryRead(playerCameraManagerBase + Offset::dw_CameraCacheEntry +Offset::dw_MinimalViewInfo + Offset::dw_Location, &myLocation, sizeof(myLocation));
    }

    float aimAnticipation2 = aimAnticipation / 100; //预判

    uintptr_t myGunBase = getA(getA(uMyAddress + Offset::weaponOffset) + Offset::gunOffset);

    if (aimAnticipation2 > 0) {
        Vector3A moveLocation;
        memoryRead(aimObjaddr + Offset::VelocityOffset, &moveLocation,sizeof(moveLocation));

        //距离
        double distance = sqrtf(powf(aimObjInfo.X - myLocation.X, 2.0f) + powf(aimObjInfo.Y - myLocation.Y, 2.0f) + powf(aimObjInfo.Z - myLocation.Z, 2.0f));
        //子弹速度
        double aimoSpeed = getF(myGunBase + Offset::bulletVelocityOffset) * 0.01f;
        //到达耗时
        double flyTime = (distance / 100.0f) / aimoSpeed; //秒
        //下坠米
        double dropM = 0.0f;
        //飞行和修改耗时
        double useS = flyTime + aimAnticipation2;
        yupanPoint.X = aimObjInfo.X + moveLocation.X * useS;
        yupanPoint.Y = aimObjInfo.Y + moveLocation.Y * useS;
        yupanPoint.Z = aimObjInfo.Z + moveLocation.Z * useS + dropM;

        double yuPanDistance = sqrtf(powf(yupanPoint.X - myLocation.X, 2.0f) + powf(yupanPoint.Y - myLocation.Y, 2.0f) +powf(yupanPoint.Z - myLocation.Z, 2.0f)) * 0.01f;
        double realFlyTime = yuPanDistance / aimoSpeed;
        double realUseS = realFlyTime + aimAnticipation2;
        yupanPoint.X = aimObjInfo.X + moveLocation.X * realUseS;
        yupanPoint.Y = aimObjInfo.Y + moveLocation.Y * realUseS;
        yupanPoint.Z = aimObjInfo.Z + moveLocation.Z * realUseS + dropM;

        double zDistance = (myLocation.Z - yupanPoint.Z) * 0.01f;
        trackData.loc = yupanPoint;
        FRotator TargetRot = ToRotator({myLocation.X, myLocation.Y, myLocation.Z},{yupanPoint.X, yupanPoint.Y, yupanPoint.Z});
        trackData.aim_angle = {TargetRot.Pitch,TargetRot.Yaw,0};
    } else {
        trackData.loc = aimObjInfo;
        FRotator TargetRot = ToRotator({myLocation.X, myLocation.Y, myLocation.Z}, {aimObjInfo.X, aimObjInfo.Y, aimObjInfo.Z});
        trackData.aim_angle = {TargetRot.Pitch,TargetRot.Yaw,0};
    }
    return trackData;
}

static bool bIsShotGun() {
    return strstr(getName(getA(uMyAddress + Offset::weaponOffset)).c_str(), "ShotGun");
}


void getPlayerName(uintptr_t nameOffset,char *buf) {
    memset(buf, '\0', 28);
    if (!isSafeAddress(nameOffset, 4))
        return;
    auto *name = reinterpret_cast<FString *>(nameOffset);
    name->ToString(buf);
}

void *updateDataList(void *) {
    prctl(PR_SET_NAME, "updateDataList");
    memset(BoneIndexs, 0, 65 * sizeof(int));
    while (true) {
        usleep(1000 * 1200);

        if (GetBoneLocation == nullptr) {
            *(uintptr_t *) &GetBoneLocation = libUE4 + Offset::GetBoneLocationOffset;
            *(uintptr_t *) &GetBoneName = libUE4 + Offset::GetBoneNameOffset;
            *(uintptr_t *) &AddControllerYawInput = libUE4 + Offset::AddYawInputOffset;
            *(uintptr_t *) &AddControllerPitchInput = libUE4 + Offset::AddPitchInputOffset;
            *(uintptr_t *) &LineOfSightTo = libUE4 + Offset::LineOfSightToOffset;
            *(uintptr_t *) &ProjectWorldLocationToScreen = libUE4 + Offset::ProjectWorldLocationToScreenOffset;
            *(uintptr_t *) &GetNumBones = libUE4 + Offset::GetNumBonesOffset;
            for (int & i : increase) {
                cacheWeapon.insert(std::pair<int, int>(i, 1));
            }
            continue;
        }

        //GName
        if (GnameAddress == 0) {
            GnameAddress = getA(libUE4 + Offset::GNames);
            continue;
        }

        //世界
        uWorldAddress = getA(libUE4 + Offset::GWorld);
        //世界资源列表
        PersistentLevel = getA(uWorldAddress + Offset::UWorldToPersistentLevel);
        // 是否在游戏大厅
        NetDriver = getA(uWorldAddress + Offset::UWorldToNetDriver);
        //更新数据时把缓存清空重新获取
        if (NetDriver == 0) {
            isRead = false;
            continue;
        }

        isRead = true;
        // 人物组件
        playerController = getA(getA(NetDriver + Offset::NetDriverToServerConnection) + Offset::ServerConnectionToPlayerController);
        if (!CameraManager::isSafePlayerController()) {
            continue;
        }
        //相机管理
        playerCameraManagerBase = getA(playerController + Offset::playerCameraManagerOffset);

        //人物对象
        uintptr_t ActorAddress = getA(PersistentLevel + Offset::ULevelToAActors);
        //数组大小
        int ResNum = getI(PersistentLevel + Offset::ULevelToAActorsCount);

        actorsCount = 0;
        memset(update, 0, (MaxActorCount + 5) * sizeof(Update));

        //读取一些基础数据
        for (int i = 0; i < ResNum; i++) {
            if (actorsCount >= MaxActorCount) {
                continue;
            }

            // 对象指针
            uintptr_t ObjectPointer = getA(ActorAddress + Offset::PointerSize * i);
            if (ObjectPointer <= 0x10000000 || ObjectPointer % 4 != 0 || ObjectPointer >= 0x10000000000)
                continue;

            //过滤内存缺页 4096 * 3页
            if (!isSafeAddress(ObjectPointer, 0x3000))
                continue;

            // 坐标指针
            uintptr_t RootCompont = getA(ObjectPointer + Offset::RootComponentOffset);
            if (RootCompont <= 0x10000000 || RootCompont % 4 != 0 || RootCompont >= 0x10000000000)
                continue;

            update[actorsCount].RootCompont = RootCompont;

            //获取对象类名
            string ItemName = getName(ObjectPointer);

            if (isContain(ItemName, "error"))
                continue;

            my_strcpy(update[actorsCount].ItemName, ItemName.c_str());
            //获取父类
            uintptr_t clazz = getA(ObjectPointer + Offset::UObjectToClassPrivate);
            string ClassPath = update[actorsCount].ItemName;

            uintptr_t superclass = getA(clazz + Offset::UStructToSuperStruct);
            int j = 0;
            while (superclass && j < 20) {
                ClassPath += ".";
                ClassPath += getName(superclass);
                superclass = getA(superclass + Offset::UStructToSuperStruct);
                j++;
            }

            if (isContain(update[actorsCount].ItemName, "Recycled")) { continue; }

            // 玩家
            if (isContain(ClassPath, "STExtraPlayerCharacter")) {

                if (isContain(ClassPath,"WerewolfBase")) {
                    my_strcpy(update[actorsCount].ClassPath,  "STExtraPlayerCharacter.WerewolfBase");
                } else if (isContain(ClassPath,"PlayerPawn_Hider")) {
                    my_strcpy(update[actorsCount].ClassPath,  "STExtraPlayerCharacter.PlayerPawn_Hider");
                } else {
                    my_strcpy(update[actorsCount].ClassPath, "STExtraPlayerCharacter");
                }
                uintptr_t mesh_ = getA(ObjectPointer + Offset::BoneAddrOffset);

                //人机判断
                update[actorsCount].isAI = getI(ObjectPointer + Offset::isBotOffset) == 1;

                //兼容内鬼模式
                update[actorsCount].isRat = isContain(ClassPath, "Impostors");

                if (!update[actorsCount].isAI && !update[actorsCount].isRat) {
                    //人物名称
                    uintptr_t NamePointer = ObjectPointer + Offset::PlayerNameOffset;
                    if (getA(NamePointer) <= 0x10000000) continue;
                    getPlayerName(NamePointer, update[actorsCount].PlayerName);
                    if (strlen(update[actorsCount].PlayerName) <= 0) {
                        continue;
                    }
                }
                //队伍信息
                update[actorsCount].TeamID = getI(ObjectPointer + Offset::TeamIDOffset);

                update[actorsCount].plater_mesh_ = mesh_;

                update[actorsCount].actorBases = ObjectPointer;

                actorsCount++;
            }

                // 载具
            else if (isContain(ClassPath, "STExtraVehicleBase") || isContain(ClassPath, "FlightVehicle")) {
                if (!isVehicle)
                    continue;
                my_strcpy(update[actorsCount].ClassPath,"STExtraVehicleBase");
                update[actorsCount].actorBases = ObjectPointer;
                actorsCount++;
            }
                // 飞机盒子空->>
            else if (isContain(ClassPath, "PickUpListWrapperActor") || isContain(ClassPath, "AirDropBoxActor")) {
                if (!showChest && !showDrop) continue;
                my_strcpy(update[actorsCount].ClassPath,"PickUpListWrapperActor");
                update[actorsCount].actorBases = ObjectPointer;
                actorsCount++;
            }
                // 武器配件子弹
            else if (isContain(ClassPath, "PickUpWrapperActor")) {
                if (!isItems)
                    continue;
                my_strcpy(update[actorsCount].ClassPath,"PickUpWrapperActor");
                update[actorsCount].actorBases = ObjectPointer;
                actorsCount++;
            }
                // 投掷物预警
            else if (isContain(ClassPath, "EliteProjectile")) {
                if (!isMissiles)
                    continue;
                my_strcpy(update[actorsCount].ClassPath,"EliteProjectile");
                update[actorsCount].actorBases = ObjectPointer;
                actorsCount++;
            }
        }

        isRead = false;
    }
}

void createDataList() {
    //读取ue4头部地址
    if (libUE4 == 0) { return; }

    //游戏分辨率
    px = glWidth / 2, py = glHeight / 2, screenSizeX = glWidth, screenSizeY = glHeight;
    //一些开关控制
    isItems = showItem;
    isPeople = true;
    isMissiles = showGrenade;
    isVehicle = showVehicle;
    my_aimedMode = aimedMode;
    record_Max_shake = max_shake;
    PlayerCount = 0;
    VehicleCount = 0;
    ItemsCount = 0;
    AirBoxCount = 0;
    GrenadeCount = 0;

    // 是否在游戏大厅
    NetDriver = getA(uWorldAddress + Offset::UWorldToNetDriver);

    if (NetDriver == 0) {
        usleep(100);
        return;
    }

    initDraw();

    playerController = getA(getA(NetDriver + Offset::NetDriverToServerConnection) + Offset::ServerConnectionToPlayerController);
    if (playerController > 0 && isGetBase) {
        uintptr_t vtable = getA(playerController + 0);
        *(uintptr_t *) &LineOfSightTo = getA(vtable + 0x818);
        *(uintptr_t *) &AddControllerPitchInput = getA(vtable + 0xD20);
        *(uintptr_t *) &AddControllerYawInput = getA(vtable + 0xD28);
        LOGD("AddControllerPitchInput:%lx", *(uintptr_t *) &AddControllerPitchInput - libUE4);
        LOGD("AddControllerYawInput:%lx", *(uintptr_t *) &AddControllerYawInput - libUE4);
        LOGD("LineOfSightTo:%lx", *(uintptr_t *) &LineOfSightTo - libUE4);
        isGetBase = false;
    }
    playerCameraManagerBase = getA(playerController + Offset::playerCameraManagerOffset);

    //自身阵营
/*    uintptr_t uMyObject = getA(playerController + Offset::uMyObjectOffset);
    int tempTid = getI(uMyObject + Offset::TeamIDOffset);
    if (tempTid < 110 && tempTid > 0) {
        SelfTeamID = tempTid;
        uMyAddress = uMyObject;// 缓存自己地址
    }*/

    //根据手持武器来判断自瞄模式 狙击枪开镜 其他 开火
    if (my_aimedMode == 3 && aimbot) {
        int fireTypeInt = getI(getA(uMyAddress + Offset::weaponOffset) + Offset::shootModeOffset);
        char fireType = (char) ((fireTypeInt >> 8) & 0xFF);
        //单发模式开镜自瞄
        my_aimedMode = fireType == 1 ? 0 : 1;
    }

    if (aimType == 1 && aimbot) {
        my_aimedMode = 2;
    }

    //读取自身开镜信息
    memoryRead(uMyAddress + Offset::adsOffset, &myIsGunADS, 1);
    if (!myIsGunADS && isKernel) {
        aimRange = closureAimRange;
    } else{
        aimRange = openAimRange;
    }

    //自瞄范围
    if (showRange && aimbot) {
        CustomImDrawList::drawCircle((int) px, (int) py, aimRange, Color.Yellow, 0, 1);
    }

    //判断自定义聚点准心是否开启
    openAccumulation = showCrosshair && !myIsGunADS && getA(getA(uMyAddress + Offset::weaponOffset) + Offset::gunOffset) != 0;

    //用于自瞄对象
    float nearest = 9999.0f;
    bIsGunADS = 0;
    bIsPressingFireBtn = 0;
    uintptr_t tempAddr = 0;

    //当前为方式数据
    isSend = true;

    //拉取更新数据
    if (actorsCount > 0 && !isRead) {
        updateActorsCount = actorsCount;
        memset(update_data, 0, updateActorsCount * sizeof(Update));
        my_memmove(update_data, update, updateActorsCount * sizeof(Update));
    }

    //相机
    Vector3A SelfInfo;
    memoryRead(playerCameraManagerBase + Offset::dw_CameraCacheEntry + Offset::dw_MinimalViewInfo + Offset::dw_Location, &SelfInfo, sizeof(SelfInfo));

    for (int i = 0; i < updateActorsCount; i++) {
        // 对象指针
        uintptr_t ObjectPointer = update_data[i].actorBases;
        if (ObjectPointer == 0) {
            continue;
        }

        // 坐标指针(控制器)
        uintptr_t RootCompont = update_data[i].RootCompont;
        if (RootCompont == 0)
            continue;

        //检查更新后的正常数据是否再次被缺页
        if (!isSafeAddress(RootCompont, Offset::PointerSize))
            continue;

        if (isContain(update_data[i].ClassPath, "STExtraPlayerCharacter")) {
            // 检测是否超过最大读取人数
            if (PlayerCount >= MaxPlayerCount)
                continue;

            if (update_data[i].plater_mesh_ == 0)
                continue;

            if (!isContain(getName(update_data[i].plater_mesh_), "CharacterMesh0"))
                continue;

            int teamID = update_data[i].TeamID;

            int Role = getI(ObjectPointer + Offset::RoleOffset);
            if (Role == 258) {
                SelfTeamID = teamID;
                uMyAddress = ObjectPointer;
                uCurrentVehicle = getA(ObjectPointer + Offset::RideVehicleOffset);
                continue;
            }

            // 排除队友
            if ((teamID == SelfTeamID || teamID <= 0) && !isContain(update_data[i].ClassPath, "WerewolfBase"))
                continue;

            PlayerData data;
            //队伍
            data.TeamID = teamID;

            /* 坐标数据 */
            Vector3A ObjInfo;
            memoryRead(RootCompont + Offset::CoordinateOffset, &ObjInfo, sizeof(ObjInfo));
            if (isnan(ObjInfo.X) || isinf(ObjInfo.X) || isnan(ObjInfo.Y) || isinf(ObjInfo.Y) || isnan(ObjInfo.Z) || isinf(ObjInfo.Z)) {continue;}

            // 血量信息
            float HelathInfo[3] = {0};
            HelathInfo[0] = getF(ObjectPointer + Offset::HealthOffset);
            HelathInfo[2] = getF(ObjectPointer + Offset::maxHealthOffset);
            data.Health = HelathInfo[0] / HelathInfo[2] * 100;

            // 人物状态
            data.State = getI(ObjectPointer + Offset::StateOffset);

            // 人物死亡 血量 = 0 且 不是倒地状态 也 不是自救状态
            if (data.State == 1048576 || data.State == 0 || data.State == 1048577 || data.State == 1048592 || data.State == 71894088)
                continue;

            // 判断人机
            data.isAI = update_data[i].isAI;

            //是否显示
            bool isShow = worldToScreen(&ObjInfo, &data.Location);

            //对象距离
            data.Distance = getDistance(ObjInfo.X, ObjInfo.Y, ObjInfo.Z, SelfInfo.X, SelfInfo.Y, SelfInfo.Z) / 100;

            if (data.Distance > 500.0)
                continue;

            //这是用于头部大小
            data.HeadSize = data.Location.w / 6.4f;

            // 对象朝向
            data.Angle = getF(ObjectPointer + Offset::RotationAngleOffset);

            data.Angle = data.Angle + 90.0f;

            if (data.Angle < 0)
                data.Angle = 360.0f + data.Angle;

            // 旋转角度_雷达
            data.RotationAngle = rotateCoord(getF(uMyAddress + Offset::RotationAngleOffset) - 90.0f,(SelfInfo.X - ObjInfo.X) / 225, (SelfInfo.Y - ObjInfo.Y) / 225);
            // 雷达坐标
            data.RadarLocation = Vector2A(ObjInfo.X - SelfInfo.X, ObjInfo.Y - SelfInfo.Y);

            //兼容内鬼模式
            data.isRat = update_data[i].isRat;

            //是否载具上
            bool isDrive = false;
            uintptr_t CurrentVehicle = getA(ObjectPointer + Offset::RideVehicleOffset);
            if (CurrentVehicle > 0){
                //抬高人物坐标
                ObjInfo.Z = ObjInfo.Z + 100;
                string ItemName = getName(CurrentVehicle);
                if (isContain(ItemName, "BP_SciFi_Moto_C") || isContain(ItemName, "VH_BRDM_C"))
                    isDrive = true;
            }

            int holdingState = getI(ObjectPointer + Offset::holdingStateOffset); //手持状态
            uintptr_t my_weapon = getA(ObjectPointer + Offset::weaponOffset);
            if (holdingState == 1 || holdingState == 2 || holdingState == 3) {
                data.Weapon.id = getI(my_weapon + Offset::weaponIDOffset); //ID
                data.Weapon.CurrentBullet = getI(my_weapon + Offset::weaponAmmoOffset); //剩余子弹
                data.Weapon.TotalBullet = getI(my_weapon + Offset::weaponClipOffset); //弹夹
            }
            //绘制雷达
            drawRadar(data.Distance, data.RotationAngle.X, data.RotationAngle.Y, data.isAI, getisDie(data.State, data.Health), radarOffsetX, radarOffsetY, data.RadarLocation.X,  data.RadarLocation.Y,  data.Angle );
            //读取玩家名称 骨骼
            if (isShow && data.Location.x > 0 && data.Location.x < px * 2 && data.Location.y > 0 && data.Location.y < py * 2 && data.Distance >= 4 && data.State != 1048576) {

                //骨骼数据
                DrawBone(update_data[i].plater_mesh_, &(data.mBoneData));

                //掩体判断
                if (isVisibility)
                    data.isVisibility = isVisiblePoint(ObjInfo);

                //人物信息
                DrawInfo(data.Health, data.TeamID, update_data[i].PlayerName, data.Distance,
                         data.isRat,
                         data.isAI,
                         data.isVisibility, getisDie(data.State, data.Health),
                         data);
            }

            PlayerCount++;
            //判断180°FOV 自瞄

            if (aimbot) {
                //开火开镜 射出 = 1 开镜 = 1||257 || 256 否则 0
                if (my_aimedMode == 2 || my_aimedMode == 1)
                    bIsPressingFireBtn = getI(playerController + Offset::fireButtonOffset);
                if (my_aimedMode == 2 || my_aimedMode == 0)
                    memoryRead(uMyAddress + Offset::adsOffset, &bIsGunADS, 1);
            }

            bool isCutObj = true;
            if (aimbot && (bIsPressingFireBtn || bIsGunADS)) {
                // 掩体不自瞄的情况下 敌人切换目标
                if (!visibilityAim && !isDrive) {
                    Vector3A aim = aimBoneLocation(update_data[i].plater_mesh_);
                    isCutObj = aim.X > 0 && aim.Y > 0;
                }
            }

            if (isCutObj && isShow && (dieNoAim || data.Health > 0) && aimbot && aimData == 0) {

                float centerDist = get2dDistance(px, py, data.Location.x, data.Location.y);

                if (centerDist < aimRange) {
                    //距离不为中心距离切换3d距离
                    if (aimChoose != 0 && aimChoose != 1)
                        centerDist = data.Distance;
                    else if (aimChoose == 1) {
                        if (data.Distance < 50) {
                            centerDist = data.Distance;
                        } else {
                            centerDist = get2dDistance(px, py, data.Location.x, data.Location.y);
                        }
                    }
                    if (nearest > centerDist) {
                        nearest = centerDist;
                        tempAddr = ObjectPointer;
                    }
                }
            }
        }

            // 载具
        else if (isContain(update_data[i].ClassPath, "STExtraVehicleBase") || isContain(update_data[i].ClassPath, "FlightVehicle")) {
            if (!isVehicle)
                continue;

            if (ObjectPointer == uCurrentVehicle)
                continue;

            if (VehicleCount >= MaxVehicleCount)
                continue;

            Vector3A ObjInfo;
            memoryRead(RootCompont + Offset::CoordinateOffset, &ObjInfo, sizeof(ObjInfo));

            if (isSafeAddr(ObjInfo))
                continue;

            ItemData obj{};
            // 对象距离
            obj.Distance = worldToScreen(&ObjInfo, &obj.Location);

            if (obj.Distance > VehicleDis || obj.Distance < 3 || obj.Location.X > px * 2 || obj.Location.Y > py * 2 || obj.Location.Y <= 0 || obj.Location.X <= 0)
                continue;

            VehicleCount++;
            ImVec4 wp_color;
            string wp_name = GetVehicleInfo(update_data[i].ItemName,&wp_color);

            //载具的驾驶状态
            bool bDriveState = getI(ObjectPointer + Offset::driveStateOffset) == 65792;

            if (!wp_name.empty()) {
                char Temp[256];
                if (bDriveState && isKernel){
                    sprintf(Temp, "! %s(%d)", wp_name.c_str(), int(obj.Distance));
                    CustomImDrawList::drawText(obj.Location.X, obj.Location.Y , ImVec4(255.f / 255.f, 0 / 255.f, 0 / 255.f, 255 / 255.f), Temp, font_draw, true);
                }else{
                    sprintf(Temp, "%s(%d)", wp_name.c_str(), int(obj.Distance));
                    CustomImDrawList::drawText(obj.Location.X, obj.Location.Y , vehicle_color, Temp, font_draw, true);
                }
            }
        }

            // 飞机盒子空->>
        else if (isContain(update_data[i].ClassPath, "PickUpListWrapperActor") || isContain(update_data[i].ClassPath, "AirDropBoxActor")) {
            if (!showChest && !showDrop)
                continue;

            if (AirBoxCount >= MaxAirBoxCount)
                continue;

            ItemData obj{};
            Vector3A ObjInfo;
            memoryRead(RootCompont + Offset::CoordinateOffset, &ObjInfo, sizeof(ObjInfo));

            if (isSafeAddr(ObjInfo))
                continue;

            // 对象距离
            obj.Distance = worldToScreen(&ObjInfo, &obj.Location);

            if (obj.Distance < 3 || obj.Location.X > px * 2 || obj.Location.Y > py * 2 || obj.Location.Y <= 0 || obj.Location.X <= 0)
                continue;

            if ((isContain(update_data[i].ItemName, "AirDropListWrapperActor") || isContain(update_data[i].ItemName, "CarePackage")) && obj.Distance < AirDis && showDrop) {
                char countTemp[256];
                sprintf(countTemp, "%s(%d)", "空投", (int) obj.Distance);
                CustomImDrawList::drawText(obj.Location.X, obj.Location.Y, Color.White, countTemp,font_draw, true);
            } else if ((isContain(update_data[i].ItemName, "PickUpListWrapperActor") || isContain(update_data[i].ItemName, "PlayerDeadInventoryBox_C")) && showChest && obj.Distance < BoxDis) {
                char countTemp[256];
                sprintf(countTemp, "%s(%d)", "盒子", (int) obj.Distance);
                CustomImDrawList::drawText(obj.Location.X, obj.Location.Y, Color.White, countTemp,font_draw, true);
            }

            obj.ItemNum = getI(ObjectPointer + Offset::IDNumberOffset); //盒子物品数量

            if (obj.ItemNum < 50 && obj.ItemNum > 0) {
                int z = 0;
                for (int j = 0; j < obj.ItemNum; j++) {
                    uintptr_t BoxAddress = getA(ObjectPointer + Offset::BoxAddressOffset) + j * Offset::BoxItemSize; //物品地址
                    int ItemID = getI(BoxAddress + 0x4); //物品ID
                    if (ItemID > 100000 && ItemID < 999999) {
                        string itemName = getBoxName(ItemID);
                        if (isContain(itemName, "Error")) {continue;}
                        if ((isContain(update_data[i].ItemName, "AirDropListWrapperActor")) && obj.Distance < 300 && showDrop && showBoxItem) {
                            z++;
                            CustomImDrawList::drawText(obj.Location.X, obj.Location.Y - z * getFontHeight(),arr[ItemID % length], itemName.c_str(),font_draw, true);
                        } else if ((isContain(update_data[i].ItemName, "PickUpListWrapperActor") || isContain(update_data[i].ItemName, "PlayerDeadInventoryBox_C")) && showChest && obj.Distance < 10 && showBoxItem) {
                            z++;
                            CustomImDrawList::drawText(obj.Location.X, obj.Location.Y - z * getFontHeight(),arr[ItemID % length], itemName.c_str(),font_draw, true);
                        }
                    }
                }
            }
            AirBoxCount++;
        }

            // 武器配件子弹
        else if (isContain(update_data[i].ClassPath, "PickUpWrapperActor")) {
            if (ItemsCount >= MaxItemsCount)
                continue;

            if (!isItems)
                continue;

            Vector3A ObjInfo;
            memoryRead(RootCompont + Offset::CoordinateOffset, &ObjInfo, sizeof(ObjInfo));
            if (isSafeAddr(ObjInfo))
                continue;

            ItemData data{};
            // 对象距离
            data.Distance = worldToScreen(&ObjInfo, &data.Location);
            if (data.Distance < 3 || data.Distance > ItemDis || data.Location.X > px * 2 || data.Location.Y > py * 2 || data.Location.Y <= 0 || data.Location.X <= 0)
                continue;
            // 对象坐标
            ItemsCount++;

            ImVec4 wp_color;
            string wp_name = GetItemInfo(update_data[i].ItemName,&wp_color);
            if (!wp_name.empty()) {
                char Temp[256];
                sprintf(Temp, "%s(%dm)", wp_name.c_str(), int(data.Distance));
                CustomImDrawList::drawText(data.Location.X, data.Location.Y, wp_color, Temp, font_draw, true);
            }
        }
            // 投掷物预警
        else if (isContain(update_data[i].ClassPath, "EliteProjectile")) {
            if (!isMissiles)
                continue;

            if (GrenadeCount >= MaxGrenadeCount)
                continue;

            Vector3A ObjInfo;
            memoryRead(RootCompont + Offset::CoordinateOffset, &ObjInfo, sizeof(ObjInfo));

            if (isSafeAddr(ObjInfo))
                continue;

            ItemData obj{};
            // 对象距离
            obj.Distance = worldToScreen(&ObjInfo, &obj.Location);
            if (strstr("ProjSmoke_BP_C", update_data[i].ItemName)) {
                float projSmoke2D = get2dDistance(px, py, obj.Location.X, obj.Location.Y - 150);
                if (projSmoke2D <= projSomoke) {
                    isMyProjSomoke = true;
                }
                if (!isProjSomoke) {
                    isMyProjSomoke = false;
                }
            } else {
                isMyProjSomoke = false;
            }

            if (obj.Distance < 3 || obj.Location.X > px * 2 || obj.Location.Y > py * 2 || obj.Location.Y <= 0 || obj.Location.X <= 0)
                continue;

            //对象名称
            ImVec4 wp_color;
            string wp_name = GetGrenadeInfo(update_data[i].ItemName, &wp_color);
            if (!wp_name.empty()) {
                char Temp[256];
                sprintf(Temp, "%s(%d)", wp_name.c_str(), int(obj.Distance));
                CustomImDrawList::drawText(obj.Location.X, obj.Location.Y, wp_color, Temp, font_draw, true);
            }
            GrenadeCount++;
        }
    }

    DrawNum(PlayerCount);

    //读取完数据挂起状态,等待被拉起
    if (aimbot && NetDriver != 0) {

        if (aimData == 0) { aimObjaddr = aimData = tempAddr; }

        if (isSafeAddress(aimData, Offset::PointerSize) && !isMyProjSomoke) {

            float hp = getF(aimData + Offset::HealthOffset);
            bool isAi = getI(aimData + Offset::isBotOffset);
            int State = getI(aimData + Offset::StateOffset);
            bool isRideVehicle = getA(aimData + Offset::RideVehicleOffset) != 0;
            if (State == 1048576 || State == 0 || State == 1048577 || State == 1048592 || State == 71894088) {
                aimData = 0;
                isHookAngle = false;
                isMyProjSomoke = false;
                isSend = false;
                return;
            }
            bool isPour = !(!dieNoAim && getisDie(State, hp));
            bool isPour1; //人机是否瞄准
            isAi ? isPour1 = aiAim : isPour1 = true;
            uintptr_t mesh_ = getA(aimData + Offset::BoneAddrOffset);
            Vector3A ObjInfo = {0, 0, 0};

            ObjInfo = getBoneLoc(mesh_, 5);
            //单发武器50米内不开火自瞄，并且切换锁定部位。
            if (my_aimedMode == 0 && isKernel) {
                record_Max_shake =  0.1;
                float distance = getDistance(ObjInfo.X, ObjInfo.Y, ObjInfo.Z, SelfInfo.X, SelfInfo.Y, SelfInfo.Z) / 100;
                if (distance <= 50) {
                    bIsPressingFireBtn = true;
                    lockPart = 1;
                } else {
                    lockPart = 3;
                }
            } else {
                lockPart = 2;
            }

            //开火和开镜时锁定自瞄对象
            if ((bIsPressingFireBtn || bIsGunADS) && isPour && isPour1) {

                if (aimData != 0) {
                    if (isSafeAddress(mesh_, Offset::PointerSize)) {

                        if (aimLocation == 0) {
                            ObjInfo = getBoneLoc(mesh_, 5);
                            ObjInfo.Z += 7;
                            bool isVisib = !(!isVisiblePoint(ObjInfo) && !visibilityAim);
                            if (!isVisib && !isRideVehicle) {
                                ObjInfo = {0, 0, 0};
                            }
                        } else if (aimLocation == 1) {
                            ObjInfo = getBoneLoc(mesh_, 4);
                            bool isVisib = !(!isVisiblePoint(ObjInfo) && !visibilityAim);
                            if (!isVisib && !isRideVehicle) {
                                ObjInfo = {0, 0, 0};
                            }
                        } else if (aimLocation == 2) {
                            ObjInfo = getBoneLoc(mesh_, 2);
                            bool isVisib = !(!isVisiblePoint(ObjInfo) && !visibilityAim);
                            if (!isVisib && !isRideVehicle) {
                                ObjInfo = {0, 0, 0};
                            }
                        } else {
                            if (lockPart == 1){//喷子默认锁腰,全自动默认锁胸。
                                //默认锁腰
                                ObjInfo = getBoneLoc(mesh_, 2);
                            }else if (lockPart == 2){
                                //默认锁胸
                                ObjInfo = getBoneLoc(mesh_, 4);
                            }else if (lockPart == 3){
                                //默认锁头
                                ObjInfo = getBoneLoc(mesh_, 5);
                            }
                            if (!(isVisiblePoint(ObjInfo) || visibilityAim) && !isRideVehicle) {
                                //漏哪打哪
                                ObjInfo = aimBoneLocation(mesh_);
                            }
                        }
                        if (ObjInfo.X == 0 || ObjInfo.Y == 0) {
                            aimData = 0; // 恢复自瞄对象
                            isMyProjSomoke = false;
                            isHookAngle = false;
                        } else {
                            if (uMyAddress > 0) {
                                uintptr_t myGunBase = getA(getA(uMyAddress + Offset::weaponOffset) + Offset::gunOffset);
                                if (myGunBase) {
                                    isHookAngle = false;
                                    setAimLocation(aimData, ObjInfo);
                                }
                            }
                        }
                    } else {
                        aimData = 0;
                        isHookAngle = false;
                        isMyProjSomoke = false;
                    }
                }
            } else {
                aimData = 0;
                isHookAngle = false;
                isMyProjSomoke = false;
            }
        } else {
            aimData = 0;
            isHookAngle = false;
            isMyProjSomoke = false;
        }

        if (!isAimSwichObject)
            aimData = 0;
    }
    isSend = false;
}

void setGameOffset(cJSON* cjson) {
    Offset::setOffset(cjson);
}

android_app * getAndroidApp() {
    return *(android_app **) (libUE4 + Offset::GActivity);
}

DisplayMetrics getGameScreenInfo() {
    int drawSizeX = *(int*)(libUE4 + Offset::GScreen);
    int drawSizeY = *(int*)(libUE4 + Offset::GScreen + 0x4);
    return {drawSizeX, drawSizeY,0};
}

int getMatrixValue() {
    if (bIsWaiting) return -1;
    return getI(getA(getA(libUE4 + Offset::GMatrix) + 0x98) + 0x750 + 0x2b0);
}
#pragma clang diagnostic pop