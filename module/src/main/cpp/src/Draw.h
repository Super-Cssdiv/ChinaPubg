#pragma once

#include <imgui.h>
#include <string.h>
#include <string>
#include <Structs.h>

using namespace std;

void drawRadar(float dis, float RotationAngleX, float RotationAngleY, bool isAI, bool isDie,float MapX,float MapY,float RadarLocationX, float RadarLocationY, float angle);

TextureInfo createTexture(const string &ImagePath);

string GetGrenadeInfo(const char *gname, ImVec4 *color);

string GetItemInfo(const char *gname, ImVec4 *color);

string GetVehicleInfo(const char *gname, ImVec4 *color);

string getBoxName(int id);

string getWeapon(int id);

void DrawInfo(float entityHealth, int TeamID, char *name, float d, bool isRat, bool isAi,bool vis, bool Die, PlayerData obj);

/* ************************** 绘制配置 ********************************** */
extern bool showHealth, showLine, showCrosshair, showDis, showInfo, showRadar;
extern bool showBone, showBox;
extern bool showItem, show556, showRifle, showMirror, showExpansion, showDrug, showArmor, showSubmachine, showSniper, showOtherParts,showSpecial,showOther;
extern bool showVehicle, showGrenade, isVisibility;
extern bool showChest, showDrop;
extern bool showTouzhi, showSandan;
extern bool aimShortDis;
extern bool showRange;
extern bool showTarget;
extern bool showNum;
extern bool isKernel;
extern bool showWeapon;
extern float radarLocation;
extern float boneWidth, boxWidth, lineWidth;
extern bool aimFire, aimOpen;
extern bool showBoxItem;
extern bool showName;
extern bool showTeam;
extern bool isAimSwichObject;
extern bool showEagleWatch;
extern ImVec4 bone_color, box_color, line_color, fallen_color
, visibility_color, ai_color
, drop_color
, aim_color
//药品
, drug_color
//背敌预警颜色
, warning_color
//载具颜色
, vehicle_color
//装备颜色
, armor_color
//狙击枪颜色
, snipe_color
//子弹颜色
, color_bullet
//冲锋枪颜色
, submachine_color
//投掷物颜色
, missile_color
//步枪
, rifle_color
//散弹枪颜色
, color_shot
//倍镜颜色
, mirror_color
//配件颜色
, expansion_color
//其他配件颜色
, otherparts_color
, radar_color
, other_color
, special_color
, info_color;
//雷达
extern bool showRadarDis;
extern int radarOffsetX ;
extern int radarOffsetY ;
extern float radarSize;
extern bool showRadarBox;
extern int radarType;
extern bool visibilityAim, dieNoAim, aiAim, isDlip, aimbot;
extern int aimLocation, aimedMode, aimChoose;
extern float openAimRange, touchSpeed, touchPress, aimPress, aimSpeed, aimAnticipation,closureAimRange;
extern float max_shake;
extern float SlideRanges;
extern float draw_scale;
extern int aimType;
extern ImFont* font_draw;
int getFontHeight();
void initDraw();
void DrawNum(int PlayerCount);
void InitTexture();
//烟雾不瞄
extern float projSomoke;
extern bool isProjSomoke, isProjSomokeRange;
extern float VehicleDis,ItemDis,BoxDis,AirDis;
extern int Fps;
extern uintptr_t libUE4;
/*************************** 绘制配置 ***********************************/
