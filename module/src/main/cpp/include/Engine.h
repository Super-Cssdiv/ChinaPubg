#include "Structs.h"

#include <Logger.h>
#include <string>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <numeric>
#include "MemoryTools.h"
#include "GameUtils.h"
#include "GameOffsets.h"

#define PI 3.1415926
float px = 0, py = 0;

float screenSizeX = 0;
float screenSizeY = 0;

uintptr_t playerCameraManagerBase;
uintptr_t playerController = 0;
uintptr_t GnameAddress = 0;
bool (*ProjectWorldLocationToScreen)(void *PlayerController, Vector3A WorldLocation,Vector2A &ScreenLocation, bool bPlayerViewportRelative) = nullptr;

class FVector {
public:
    FVector() : x(0.f), y(0.f), z(0.f) {

    }

    FVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {

    }

    ~FVector() {

    }

    float x;
    float y;
    float z;

    inline float Dot(FVector v) {
        return x * v.x + y * v.y + z * v.z;
    }

    inline float Distance2D(float x1, float y1) {
        return (float) (sqrt(pow(double(x - x1), 2.0) + pow(double(y - y1), 2.0)));
    }

    inline float Distance(FVector v) {
        return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
    }

    inline float Length() {
        return sqrtf(x * x + y * y + z * z);
    }

    inline FVector &operator+=(const FVector &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    inline FVector &operator-=(const FVector &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    inline FVector &operator*=(const FVector &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    inline FVector &operator/=(const FVector &v) {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    inline FVector &operator+=(float v) {
        x += v;
        y += v;
        z += v;
        return *this;
    }

    inline FVector &operator-=(float v) {
        x -= v;
        y -= v;
        z -= v;
        return *this;
    }

    inline FVector &operator*=(float v) {
        x *= v;
        y *= v;
        z *= v;
        return *this;
    }

    inline FVector &operator/=(float v) {
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    inline FVector operator-() const {
        return FVector(-x, -y, -z);
    }

    inline FVector operator+(const FVector &v) const {
        return FVector(x + v.x, y + v.y, z + v.z);
    }

    inline FVector operator-(const FVector &v) const {
        return FVector(x - v.x, y - v.y, z - v.z);
    }

    inline FVector operator*(const FVector &v) const {
        return FVector(x * v.x, y * v.y, z * v.z);
    }

    inline FVector operator/(const FVector &v) const {
        return FVector(x / v.x, y / v.y, z / v.z);
    }

    inline FVector operator+(float v) const {
        return FVector(x + v, y + v, z + v);
    }

    inline FVector operator-(float v) const {
        return FVector(x - v, y - v, z - v);
    }

    inline FVector operator*(float v) const {
        return FVector(x * v, y * v, z * v);
    }

    inline FVector operator/(float v) const {
        return FVector(x / v, y / v, z / v);
    }
};

class FRotator {
public:
    FRotator() : Pitch(0.f), Yaw(0.f), Roll(0.f) {

    }

    FRotator(float _Pitch, float _Yaw, float _Roll) : Pitch(_Pitch), Yaw(_Yaw), Roll(_Roll) {

    }

    ~FRotator() {

    }

    float Pitch;
    float Yaw;
    float Roll;

    inline FRotator Clamp() {

        if (Pitch > 180) {
            Pitch -= 360;
        } else {
            if (Pitch < -180) {
                Pitch += 360;
            }
        }
        if (Yaw > 180) {
            Yaw -= 360;
        } else {
            if (Yaw < -180) {
                Yaw += 360;
            }
        }
        if (Pitch > 89) {
            Pitch = 89;
        }
        if (Pitch < -89) {
            Pitch = -89;
        }
        while (Yaw < 180) {
            Yaw += 360;
        }
        while (Yaw > 180) {
            Yaw -= 360;
        }
        Roll = 0;
        return FRotator(Pitch, Yaw, Roll);
    }

    inline float Length() {
        return sqrtf(Pitch * Pitch + Yaw * Yaw + Roll * Roll);
    }

    FRotator operator+(FRotator v) {
        return FRotator(Pitch + v.Pitch, Yaw + v.Yaw, Roll + v.Roll);
    }

    FRotator operator-(FRotator v) {
        return FRotator(Pitch - v.Pitch, Yaw - v.Yaw, Roll - v.Roll);
    }
};

class CameraManager {
public:
    static FVector GetLocation() {
        FVector location;
        memoryRead(playerCameraManagerBase + Offset::dw_CameraCacheEntry + Offset::dw_MinimalViewInfo + Offset::dw_Location,&location, sizeof(location));
        return location;
    }

    static FVector GetRotation() {
        FVector location;
        memoryRead(playerCameraManagerBase + Offset:: dw_CameraCacheEntry + Offset::dw_MinimalViewInfo + Offset::dw_Rotation,&location, sizeof(location));
        return location;
    }

    static float GetFov() {
        return getF(playerCameraManagerBase + Offset:: dw_CameraCacheEntry + Offset::dw_MinimalViewInfo + Offset::dw_FOV);
    }

    static bool isContain(const std::string &str, const char *check) {
        if (check == nullptr || str.empty()) return false;
        size_t found = str.find(check);
        return (found != std::string::npos);
    }

    static bool isSafePlayerController(){
        uintptr_t plater = getA(playerController + Offset::uMyObjectOffset - 0x10);
        if (!plater)return false;
        uintptr_t Class = getA(plater + Offset::UObjectToClassPrivate);
        if (!Class)return false;
        uintptr_t superStruct = getA(plater + Offset::UStructToSuperStruct);
        if (!superStruct)return false;
        string ClassPath = getName(superStruct);
        return isContain(ClassPath, "PlayerController") || isContain(ClassPath, "Liangzicontroller");
    }

    static bool UE4WorldLocationToScreen(Vector3A Enemy, Vector2A &ScreenLocation) {
#ifdef __LP64__
        if (playerController <= 0x10000000 || playerController % 4 != 0 || playerController >= 0x10000000000) {
            return false;
        }
#else
        if (playerController <= 0x100000 || playerController % 4 != 0 || playerController >= 0x10000000) {
            return false;
        }
#endif
        if (!isSafePlayerController()) {
            return false;
        }

        if (ProjectWorldLocationToScreen != nullptr) {
            ProjectWorldLocationToScreen(reinterpret_cast<void *>(playerController), Enemy, ScreenLocation, false);
            if ((ScreenLocation.X <= screenSizeX || ScreenLocation.Y <= screenSizeY) && ScreenLocation.X > 0 && ScreenLocation.Y > 0) {
                return true;
            }
        }
        return false;
    }

    static bool WorldToScreen(FVector WorldLocation, FVector *pOut) {
        Vector2A Location {0,0};
        UE4WorldLocationToScreen( {WorldLocation.x,WorldLocation.y,WorldLocation.z}, Location);
        pOut->x = Location.X, pOut->y = Location.Y;
        if (Location.X <= screenSizeX || Location.Y <= screenSizeY && Location.X > 0 && Location.Y > 0) {
            return true;
        }
        return false;
        /*FVector Rotation = GetRotation();
        FVector vAxisX, vAxisY, vAxisZ;
        GetAxes(Rotation, &vAxisX, &vAxisY, &vAxisZ);
        FVector vDelta = WorldLocation - GetLocation();
        FVector vTransformed = FVector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

        if (vTransformed.z < 1.f) {
            return 0;
        }
        float FovAngle = GetFov();
        pOut->x = (screenSizeX / 2.0f) + vTransformed.x * ((screenSizeX / 2.0f) / tanf(FovAngle * (float) PI / 360.f)) / vTransformed.z;
        pOut->y = (screenSizeY / 2.0f) - vTransformed.y * ((screenSizeX / 2.0f) / tanf(FovAngle * (float) PI / 360.f)) / vTransformed.z;
        if (pOut->x > 0 && pOut->y > 0) {
            if (pOut->x <= (screenSizeX / 2.0f) * 2.0f) {
                return 1;
            }
        }
        return 0;*/
    }


private:
    static void GetAxes(FVector Rotation, FVector *X, FVector *Y, FVector *Z) {
        float radPitch = (Rotation.x * float(PI) / 180.f);
        float radYaw = (Rotation.y * float(PI) / 180.f);
        float radRoll = (Rotation.z * float(PI) / 180.f);

        float SP = sinf(radPitch);
        float CP = cosf(radPitch);
        float SY = sinf(radYaw);
        float CY = cosf(radYaw);
        float SR = sinf(radRoll);

        float cro = cosf(radRoll);

        X->x = CP * CY;
        X->y = CP * SY;
        X->z = SP;

        Y->x = SR * SP * CY - cro * SY;
        Y->y = SR * SP * SY + cro * CY;
        Y->z = -SR * CP;

        Z->x = -(cro * SP * CY + SR * SY);
        Z->y = CY * SR - cro * SP * SY;
        Z->z = cro * CP;
    }
};

float get3dDistance(FVector self, FVector object, float divice) {
    FVector xyz;
    xyz.x = self.x - object.x;
    xyz.y = self.y - object.y;
    xyz.z = self.z - object.z;
    return sqrt(pow(xyz.x, 2) + pow(xyz.y, 2) + pow(xyz.z, 2)) / divice;
}

float rotateAngle(FVector selfCoord, FVector targetCoord) {
    float osx = targetCoord.x - selfCoord.x;
    float osy = targetCoord.y - selfCoord.y;
    return (float) (atan2(osy, osx) * 180 / M_PI);
}

FRotator rotateAngleView(FVector selfCoord, FVector targetCoord) {
    float osx = targetCoord.x - selfCoord.x;
    float osy = targetCoord.y - selfCoord.y;
    float osz = targetCoord.z - selfCoord.z;
    return FRotator((float) (atan2(osz, sqrt(osx * osx + osy * osy)) * 180 / M_PI), (float) (atan2(osy, osx) * 180 / M_PI), 0);
}