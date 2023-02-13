//
// Created by admin on 2021/11/16.
//
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"

#include "Draw.h"
#include <jni.h>
#include <android/log.h>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/system_properties.h>
#include <string>
#include <Logger.h>
#include <sys/mman.h>
#include <imgui.h>
#include <cstring>
#include <string>
#include <GLES3/gl3.h>
#include <ctgmath>
#include <imgui_internal.h>
#include <dirent.h>
#include <stb_image.h>
#include <imgui_expand.h>
#include <files.h>
#include <base64/base64.h>


using namespace std;
static TextureInfo textureInfo;
extern int glWidth, glHeight;
extern int screenWidth;
extern bool openAccumulation;
extern float px, py;
extern TextureInfo back1;
extern TextureInfo back2;

TextureInfo back1;
TextureInfo back2;
ImFont* font_windows;
ImFont* font_draw;

const char* strstri(const char* str, const char* subStr) {
    int len = strlen(subStr);
    if (len == 0) {
        return nullptr;
    }

    while(*str) {
        if (strncasecmp(str, subStr,len) == 0) {
            return str;
        }
        ++str;
    }
    return nullptr;
}

TextureInfo createTexture(const string &ImagePath) {
    int w, h, n;
    stbi_uc *data = stbi_load(ImagePath.c_str(), &w, &h, &n, 0);
    GLuint texture;
    glGenTextures(1, &texture);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // if (n == 3) {
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    // } else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //}
    stbi_image_free(data);
    textureInfo.textureId = (GLuint *) texture;
    textureInfo.width = w;
    textureInfo.height = h;
    return textureInfo;
}

static bool isVisible(float z) {
    return z != 0;
}


static void DrawRadar(float Angle, float x, float y, ImVec4 color) {
    float w = 45.0f / 2 / 1.5f;
    float b = 22.5f;
    if (Angle <= b || Angle >= 360 - b) {
        CustomImDrawList::drawLine(x - w, y + w, x, y - w, color, 1);
        CustomImDrawList::drawLine(x, y - w, x + w, y + w, color, 1);
    } else if (Angle >= 90 - b && Angle <= 90 + b) {
        CustomImDrawList::drawLine(x - w, y - w, x + w, y, color, 1);
        CustomImDrawList::drawLine(x + w, y, x - w, y + w, color, 1);
    } else if (Angle >= 180 - b && Angle <= 180 + b) {
        CustomImDrawList::drawLine(x - w, y - w, x, y + w, color, 1);
        CustomImDrawList::drawLine(x, y + w, x + w, y - w, color, 1);
    } else if (Angle >= 270 - b && Angle <= 270 + b) {
        CustomImDrawList::drawLine(x + w, y - w, x - w, y, color, 1);
        CustomImDrawList::drawLine(x - w, y, x + w, y + w, color, 1);
    } else if (Angle >= 45 - b && Angle <= 45 + b) {
        CustomImDrawList::drawLine(x + w, y - w, x - w, y, color, 1);
        CustomImDrawList::drawLine(x + w, y - w, x, y + w, color, 1);
    } else if (Angle >= 135 - b && Angle <= 135 + b) {
        CustomImDrawList::drawLine(x + w, y + w, x - w, y, color, 1);
        CustomImDrawList::drawLine(x + w, y + w, x, y - w, color, 1);
    } else if (Angle >= 225 - b && Angle <= 225 + b) {
        CustomImDrawList::drawLine(x - w, y + w, x, y - w, color, 1);
        CustomImDrawList::drawLine(x - w, y + w, x + w, y, color, 1);
    } else if (Angle >= 315 - b && Angle <= 315 + b) {
        CustomImDrawList::drawLine(x - w, y - w, x + w, y, color, 1);
        CustomImDrawList::drawLine(x - w, y - w, x, y + w, color, 1);
    }

}




string GetGrenadeInfo(const char *gname ,ImVec4 *color) {
    string name;
    if (strstr(gname, "ProjGrenade_BP_C") != nullptr)//手雷
    {
        name = "小心手雷";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    if (strstr(gname, "ProjSmoke_BP_C") != nullptr) {
        name = "有人投掷烟雾";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    if (strstr(gname, "ProjBurn_BP_C") != nullptr) {
        name = "小心燃烧瓶";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    if (strstr(gname, "BP_Grenade_Pan_C") != nullptr) {
        name = "小心平底锅";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    return "";
}


int getFontHeight() {
    ImVec2 text_size = font_draw->CalcTextSizeA(font_draw->FontSize, FLT_MAX, -1.0f, "Hello World");
    return (int)text_size.y;
}

string getBoxName(int id) {
    if (id == 601006) return "[药品]医疗箱";
    if (id == 601005) return "[药品]急救包";
    if (id == 601004) return "[药品]绷带";
    if (id == 601001) return "[药品]饮料";
    if (id == 601002) return "[药品]强力针";
    if (id == 601003) return "[药品]止痛药";


    if (id == 503001) return "[防具]一级防弹衣";
    if (id == 503002) return "[防具]二级防弹衣";
    if (id == 503003) return "[防具]三级防弹衣";
    if (id == 502001) return "[防具]一级头盔";
    if (id == 502002) return "[防具]二级头盔";
    if (id == 502003) return "[防具]三级头盔";
    if (id == 501001) return "[背包]一级包";
    if (id == 501002) return "[背包]二级包";
    if (id == 501006) return "[背包]三级包";


    if (id == 107001) return "[武器]十字弩";
    if (id == 103003) return "[武器]AWM";
    if (id == 103001) return "[武器]Kar98K";
    if (id == 105002) return "[武器]DP28";
    if (id == 103002) return "[武器]M24";
    if (id == 101005) return "[武器]狗砸";
    if (id == 101001) return "[武器]AKM";
    if (id == 106005) return "[武器]R45";
    if (id == 101009) return "[武器]MK47";
    if (id == 101006) return "[武器]AUG";
    if (id == 103006) return "[武器]Mini14";
    if (id == 101002) return "[武器]M16A4";
    if (id == 101003) return "[武器]SCAR";
    if (id == 102001) return "[武器]UZI";
    if (id == 102004) return "[武器]汤姆逊冲锋枪";
    if (id == 102003) return "[武器]维克托";
    if (id == 102002) return "[武器]UMP9";
    if (id == 103005) return "[武器]VSS射手步枪";
    if (id == 103008) return "[武器]Win94";
    if (id == 103009) return "[武器]SLR";
    if (id == 105001) return "[武器]QBU";
    if (id == 101007) return "[武器]QBZ";
    //if(id==105001,return "[武器]大菠萝";
    if (id == 106003) return "[武器]R1895";
    if (id == 101004) return "[武器]M416";
    if (id == 106006) return "[武器]短管散弹枪";
    if (id == 104003) return "[武器]S12K";
    if (id == 104002) return "[武器]S1897";
    if (id == 104001) return "[武器]双管猎枪";
    if (id == 102105) return "[武器]P90冲锋枪";
    if (id == 103012) return "[武器]AMR狙击枪";

    if (id == 306001) return "[子弹]马格南";
    if (id == 302001) return "[子弹]762MM";
    if (id == 303001) return "[子弹]5.56MM";
    if (id == 301001) return "[子弹]9MM";
    if (id == 304001) return "[子弹]12口径";
    if (id == 305001) return "[子弹]45口径";
    if (id == 301002) return "[子弹]5.7MM";
    if (id == 306002) return "[子弹]50口径";
    if (id == 308001) return "[子弹]信号枪";
    if (id == 307001) return "[子弹]箭矢";


    if (id == 203001) return "[倍镜]红点";
    if (id == 203002) return "[倍镜]全息";
    if (id == 203003) return "[倍镜]2倍镜";
    if (id == 203014) return "[倍镜]3倍镜";
    if (id == 203004) return "[倍镜]4倍镜";
    if (id == 203015) return "[倍镜]6倍镜";
    if (id == 203005) return "[倍镜]8倍镜";


    if (id == 204014) return "[配件]子弹袋";
    if (id == 205004) return "[配件]箭袋";
    if (id == 204010) return "[配件]子弹袋";
    if (id == 202007) return "[配件]激光瞄准器";
    if (id == 202004) return "[配件]拇指握把";
    if (id == 202005) return "[配件]半截握把";
    if (id == 205001) return "[配件]垂直握把";
    if (id == 205003) return "[配件]狙击枪托";
    if (id == 205002) return "[配件]步枪枪托";
    //if(id==205001,return "[配件]枪托";
    if (id == 201003) return "[配件]狙击枪补偿器";
    if (id == 201005) return "[配件]狙击枪消焰器";
    if (id == 201007) return "[配件]狙击枪消音器";
    if (id == 201011) return "[配件]步枪消音器";
    if (id == 201009) return "[配件]步枪补偿器";
    if (id == 201010) return "[配件]步枪消焰器";
    if (id == 201006) return "[配件]冲锋枪消音器";
    if (id == 201004) return "[配件]冲锋枪消焰器";
    if (id == 201002) return "[配件]冲锋枪消焰器";
    if (id == 204009) return "[配件]狙击枪快速扩";
    if (id == 204007) return "[配件]狙击枪扩容";
    if (id == 204008) return "[配件]狙击枪快速";
    if (id == 204013) return "[配件]步枪快速扩容";
    if (id == 204011) return "[配件]步枪扩容";
    if (id == 204012) return "[配件]步枪快速";
    if (id == 204006) return "[配件]冲锋枪快速扩容";
    if (id == 204004) return "[配件]冲锋枪扩容";
    if (id == 204005) return "[配件]冲锋枪快速";
    if (id == 204003) return "[配件]手枪快扩";
    if (id == 204002) return "[配件]手枪快速";
    if (id == 204001) return "[配件]手枪扩容";


    if (id == 602003) return "[投掷]燃烧瓶";
    if (id == 602002) return "[投掷]烟雾弹";
    if (id == 602001) return "[投掷]手雷";
    if (id == 603001) return "[汽油]汽油";
    if (id == 403990) return "[衣服]吉利服";
    if (id == 403187) return "[衣服]吉利服";
    return "Error";
}

void drawRadar(float dis, float RotationAngleX,float RotationAngleY, bool isAI, bool isDie, float MapX, float MapY, float RadarLocationX,float RadarLocationY, float angle) {
    // 绘制雷达
    if (!showRadar) {
        return;
    }
    ImVec4 LC = {0, 0, 0, 0};
    if (isDie) {
        LC = fallen_color;
    }  else if(isAI) {
        LC = ai_color;
    } else {
        LC = radar_color;
    }

    if (dis > 0 && dis < 450) {

        float Offset_Maps[2] = {0, 0};

        Offset_Maps[0] = MapX;
        Offset_Maps[1] = MapY;

        if (radarType == 1) {
            float proportion = radarSize / 100;
            float MapSize = round(265 * proportion) / 2;

            float RadarLocation_X = RadarLocationX / (62.5f * 2.5f / proportion);
            float RadarLocation_Y = RadarLocationY / (62.5f * 2.5f / proportion);

            if (showRadarBox) {
                float my_height = MapSize;
                float my_width = MapSize;
                float off = MapSize / 2;

                CustomImDrawList::drawLine(0 + Offset_Maps[0] - off, 0 + Offset_Maps[1] - off,my_width + Offset_Maps[0] - off,my_height + Offset_Maps[1] - off, Color.Yellow, 1);

                CustomImDrawList::drawLine(my_width + Offset_Maps[0] - off, 0 + Offset_Maps[1] - off,0 + Offset_Maps[0] - off,my_height + Offset_Maps[1] - off, Color.Yellow, 1);

                CustomImDrawList::drawRect(0 + Offset_Maps[0] - off, 0 + Offset_Maps[1] - off,my_width + Offset_Maps[0] - off,my_height + Offset_Maps[1] - off, Color.Black, 1);

                CustomImDrawList::drawRect(my_width / 2 - 6 + Offset_Maps[0] - off,my_height / 2 - 6 + Offset_Maps[1] - off,my_width / 2 + 6 + Offset_Maps[0] - off,my_height / 2 + 6 + Offset_Maps[1] - off,Color.Black, 1);
            }

            if (RadarLocation_X < (-MapSize / 2.0f + 45.0f / 2.0f) || RadarLocation_Y < (-MapSize / 2.0f + 45.0f / 2.0f) || RadarLocation_X > (MapSize / 2.0f - 45.0f / 2.0f) || RadarLocation_Y > (MapSize / 2.0f - 45.0f / 2.0f)) {
                float x1 = abs(RadarLocation_X);
                float y1 = abs(RadarLocation_Y);
                float z1 = max(x1, y1) / ((MapSize / 2) - (45.0f / 2.0f));
                RadarLocation_X = RadarLocation_X / z1;
                RadarLocation_Y = RadarLocation_Y / z1;
            }
            DrawRadar(angle, Offset_Maps[0] + RadarLocation_X, Offset_Maps[1] + RadarLocation_Y,LC);
            CustomImDrawList::drawRectFilled(Offset_Maps[0] + RadarLocation_X - 7.5f,Offset_Maps[1] + RadarLocation_Y - 7.5f,Offset_Maps[0] + RadarLocation_X + 7.5f ,Offset_Maps[1] + RadarLocation_Y + 7.5f, LC);
            if (showRadarDis) {
                string temp;
                if (dis > 9 && dis < 100) {
                    temp = " " + to_string((int) ceil(dis));
                } else if (dis < 10) {
                    temp = "  " + to_string((int) ceil(dis));
                } else {
                    temp = to_string((int) ceil(dis));
                }
                //DrawText(temp,Offset_Maps[0] + RadarLocation_X - 20 ,Offset_Maps[1] + RadarLocation_Y - 17, info_color);
                CustomImDrawList::drawText(temp.c_str(),Offset_Maps[0] + RadarLocation_X - 13 ,Offset_Maps[1] + RadarLocation_Y - 10, info_color);
            }
        } else {
            float ox = 0;//ImGui::GetWindowPos().x;
            float oy = 0;//ImGui::GetWindowPos().y;
            CustomImDrawList::drawCircle(Offset_Maps[0] + ox, Offset_Maps[1] + oy, 140, Color.White, 0, 3);
            CustomImDrawList::drawCircle(Offset_Maps[0]+ ox, Offset_Maps[1] + oy, 30, Color.White, 0, 3);
            if (showRadarDis) {
                string temp = to_string((int) dis) + "m";
                CustomImDrawList::drawText(temp.c_str(), RotationAngleX + 40 + Offset_Maps[0]+ ox,RotationAngleY + Offset_Maps[1] + oy, info_color);
            }
            CustomImDrawList::drawCircleFilled(RotationAngleX + Offset_Maps[0]+ ox, RotationAngleY + Offset_Maps[1]+oy,10, LC, 0);
            CustomImDrawList::drawCircleFilled(Offset_Maps[0]+ ox, Offset_Maps[1]+oy, 10, Color.Yellow, 0);

        }
    }
}

void initDraw() {
    //十字准心
    if (openAccumulation && showCrosshair) {
        int radarLeft = px, radarTop = py;
        CustomImDrawList::drawLine(radarLeft - 40 + 15, radarTop, radarLeft + 40 - 15, radarTop, Color.White, 1);
        CustomImDrawList::drawLine(radarLeft, radarTop - 40 + 15, radarLeft, radarTop + 40 - 15, Color.White, 1);
    }

    if (isProjSomokeRange && isProjSomoke) {
        CustomImDrawList::drawRectFilled(px - projSomoke, py - projSomoke, px + projSomoke,py + projSomoke, Color.Red_);
    }
}

void DrawNum(int PlayerCount) {
    if (!showNum) return;
    char countTemp[12];
    sprintf(countTemp, "%d", PlayerCount);
    if (PlayerCount == 0) {
        CustomImDrawList::drawImage(px - 95, py / 10 + 10, 180, 40, back2.textureId);
        CustomImDrawList::drawText(px - 18, py / 10 + 18, Color.White, "安全");
    } else {
        CustomImDrawList::drawImage(px - 95, py / 10 + 10 , 180, 40, back1.textureId);
        CustomImDrawList::drawText(px - 10, py / 10 + 18, Color.White, countTemp);
    }
}

void drawBoneLine(Vector3A start, Vector3A end, bool Die, bool isAi) {
    ImVec4 BC = {0, 0, 0, 0};
    if ((isVisible(start.Z) || isVisible(end.Z)) || !isVisibility) {
        if (Die) {
            BC = fallen_color;
        } else if (isAi) {
            BC = ai_color;
        } else {
            BC = bone_color;
        }
    }else {
        if (Die) {
            BC = fallen_color;
        } else {
            BC = visibility_color;
        }
    }
    CustomImDrawList::drawLine(start.X, start.Y, end.X, end.Y, BC, boneWidth);
}

void DrawBox(PlayerData obj, bool vis, bool Die, bool isAi) {
    if (!showBox) {
        return;
    }
    ImVec4 BC = {0, 0, 0, 0};
    if (vis || !isVisibility) {
        if (Die) {
            BC = fallen_color;
        } else if (isAi) {
            BC = ai_color;
        } else {
            BC = box_color;
        }
    }else {
        if (Die) {
            BC = fallen_color;
        } else {
            BC = visibility_color;
        }
    }
    BoneData mBoneData = obj.mBoneData;
    if (mBoneData.Left_Ankle.Y > mBoneData.Right_Ankle.Y) {
        CustomImDrawList::drawRect(mBoneData.Pelvis.X - obj.Location.w / 2 , mBoneData.Head.Y - obj.HeadSize - 5, mBoneData.Pelvis.X + obj.Location.w / 2, mBoneData.Left_Ankle.Y + 5, BC, boxWidth);
    } else {
        CustomImDrawList::drawRect(mBoneData.Pelvis.X - obj.Location.w / 2, mBoneData.Head.Y  - obj.HeadSize - 5, mBoneData.Pelvis.X + obj.Location.w / 2, mBoneData.Right_Ankle.Y + 5, BC, boxWidth);
    }
}

void DrawBone(PlayerData obj,bool isDie) {
    if (!showBone) {
        return;
    }
    //绘制骨骼
    BoneData bones = obj.mBoneData;
    //头
    ImVec4 head_color = {0, 0, 0, 0};
    if ((isVisible(bones.Head.Z) || isVisible(bones.Head.Z)) || !isVisibility) {
        if (isDie) {
            head_color = fallen_color;
        } else if (obj.isAI) {
            head_color = ai_color;
        } else {
            head_color = bone_color;
        }
    }else {
        if (isDie) {
            head_color = fallen_color;
        } else {
            head_color = visibility_color;
        }
    }
    CustomImDrawList::drawCircle(bones.Head.X, bones.Head.Y, obj.HeadSize, head_color, 0, boneWidth);
    drawBoneLine(bones.vNeck, bones.Chest, isDie, obj.isAI);
    drawBoneLine(bones.Chest, bones.Pelvis, isDie, obj.isAI);
    drawBoneLine(bones.vNeck, bones.Left_Shoulder, isDie, obj.isAI);
    drawBoneLine(bones.vNeck, bones.Right_Shoulder, isDie, obj.isAI);
    drawBoneLine(bones.Left_Shoulder, bones.Left_Elbow, isDie, obj.isAI);
    drawBoneLine(bones.Left_Elbow, bones.Left_Wrist, isDie, obj.isAI);
    drawBoneLine(bones.Right_Shoulder, bones.Right_Elbow, isDie, obj.isAI);
    drawBoneLine(bones.Right_Elbow, bones.Right_Wrist, isDie, obj.isAI);
    drawBoneLine(bones.Pelvis, bones.Left_Thigh, isDie, obj.isAI);
    drawBoneLine(bones.Pelvis, bones.Right_Thigh, isDie, obj.isAI);
    drawBoneLine(bones.Left_Thigh, bones.Left_Knee, isDie, obj.isAI);
    drawBoneLine(bones.Left_Knee, bones.Left_Ankle, isDie, obj.isAI);
    drawBoneLine(bones.Right_Thigh, bones.Right_Knee, isDie, obj.isAI);
    drawBoneLine(bones.Right_Knee, bones.Right_Ankle, isDie, obj.isAI);

}

float get2dDistance(float x, float y, float x1, float y1);

void DrawInfo(float entityHealth, int TeamID, char * name, float d, bool isRat, bool isAi,bool vis, bool Die, PlayerData obj) {

    float width = glWidth, height = glHeight;
    //血量
    string BottomText;
    if (showHealth) {
        BottomText.append(to_string((int)entityHealth)).append("HP ");
    }

    if (showDis) {
        BottomText.append(to_string((int)d)).append("M ");
    }

    if (showWeapon) {
        BottomText.append(getWeapon(obj.Weapon.id));
        if (isKernel && obj.Weapon.TotalBullet > 1) {
            BottomText.append("(").append(to_string(obj.Weapon.CurrentBullet)).append("/").append(to_string(obj.Weapon.TotalBullet)).append(")");
        }
    }

    //信息
    BoneData mBoneData = obj.mBoneData;
    string TopText;

    if (showTeam) {
        TopText.append(to_string(TeamID)).append("队 ");
    }

    if (showInfo) {
        if (isAi || isRat) {
            TopText.append(isAi ? "人机" : "内鬼");
        }
    }

    if (showName) {
        if (!isAi && !isRat) {
            TopText.append(name);
        }
    }

    if (TopText.length() != 0) {
        CustomImDrawList::drawText(TopText.c_str(),mBoneData.Pelvis.X, mBoneData.Head.Y - 28 - obj.HeadSize, info_color,font_draw,true);
    }

    if (BottomText.length() != 0) {
        if (mBoneData.Left_Ankle.Y > mBoneData.Right_Ankle.Y) {
            CustomImDrawList::drawText(BottomText.c_str(), mBoneData.Pelvis.X, mBoneData.Left_Ankle.Y + 6 , info_color,font_draw,true);
        } else {
            CustomImDrawList::drawText(BottomText.c_str(), mBoneData.Pelvis.X , mBoneData.Right_Ankle.Y + 6 , info_color,font_draw,true);
        }
    }

    //绘制射线
    if (showLine) {
        ImVec4 BC = {0, 0, 0, 0};
        if (vis || !isVisibility) {
            if (Die) {
                BC = fallen_color;
            } else if (isAi) {
                BC = ai_color;
            } else {
                BC = line_color;
            }
        }else {
            if (Die) {
                BC = fallen_color;
            } else {
                BC = visibility_color;
            }
        }
        CustomImDrawList::drawLine(px, py / 10 + 50, mBoneData.Head.X, mBoneData.Head.Y - obj.HeadSize - 22, BC, lineWidth);
    }

    //方框
    DrawBox(obj, vis, Die, isAi);
    //骨骼
    DrawBone(obj, Die);
}

void InitTexture() {
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsLight();

    //Imgui添加字体
    size_t length = (files::fontDataBase64.length( ) + 1) / 4 * 3;
    unsigned char *fontData = base64_decode((unsigned char *) files::fontDataBase64.c_str( ));
    font_windows = io.Fonts->AddFontFromMemoryTTF((void *) fontData, length, 24.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull( ));
    font_draw = io.Fonts->AddFontFromMemoryTTF((void *) fontData, length, 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull( ));

    //人数背景
    length = (files::countDataBase64.length() + 1) / 4 * 3;
    unsigned char * data = base64_decode((unsigned char *) files::countDataBase64.c_str( ));
    back1 = ImGui::loadTextureFromMemory(data, length);
    free(data);

    //安全背景
    length = (files::safeDataBase64.length() + 1) / 4 * 3;
    unsigned char * data2 = base64_decode((unsigned char *) files::safeDataBase64.c_str( ));
    back2 = ImGui::loadTextureFromMemory(data2, length);
    free(data2);
}

string GetItemInfo(const char *gname, ImVec4 *color) {
    string name;
    if (show556) {
        if (strcmp(gname, "BP_Ammo_556mm_Pickup_C") == 0) {
            name = "5.56MM";
            *color = color_bullet;
            return name;
        }
        if (strcmp(gname, "BP_Ammo_762mm_Pickup_C") == 0) {
            name = "7.62MM";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_45AC") != nullptr) {
            name = "45ACP";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_9mm") != nullptr) {
            name = "9mm";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_12Guage") != nullptr) {
            name = "12 Guage";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_300Magnum") != nullptr) {
            name = "300马格南";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_Bolt") != nullptr) {
            name = "箭矢";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "BP_Ammo_50BMG_Pickup_C") != nullptr) {
            name = "AWR子弹";
            *color = color_bullet;
            return name;
        }
    }

    //散弹枪
    if (showSandan) {
        if (strstr(gname, "S12K") != nullptr) {
            name = "S12K";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "DBS") != nullptr) {
            name = "DBS散弹枪";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "S686") != nullptr) {
            name = "S686";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "S1897") != nullptr) {
            name = "S1897";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "DP12") != nullptr) {
            name = "DBS";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "BP_ShotGun_SPAS-12_Wrapper_C") != nullptr) {
            name = "SPAS_12";
            *color = color_shot;
            return name;
        }
    }
    //投掷物
    if (showTouzhi) {
        if (strstr(gname, "Grenade_Stun") != nullptr) {
            name = "闪光弹";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Grenade_Shoulei") != nullptr) {
            name = "手榴弹";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Grenade_Smoke") != nullptr) {
            name = "烟雾弹";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Grenade_Burn") != nullptr) {
            name = "燃烧瓶";
            *color = missile_color;
            return name;
        }
        if (strcmp(gname, "BP_Grenade_Weapon_Wrapper_Thermite_C") == 0) {
            name = "铝热弹";
            *color = missile_color;
            return name;
        }
        //刀
        if (strcmp(gname, "BP_WEP_Pan_Pickup_C") == 0) {
            name = "平底锅";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Sickle") != nullptr) {
            name = "镰刀";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Machete") != nullptr) {
            name = "砍刀";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Cowbar") != nullptr) {
            name = "铁橇";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "CrossBow") != nullptr) {
            name = "十字弩";
            *color = missile_color;
            return name;
        }

    }

    //显示步枪img
    if (showRifle) {
        if (strcmp(gname, "BP_Rifle_AKM_Wrapper_C") == 0) {
            name = "AKM";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_M416_Wrapper_C") == 0) {
            name = "M416";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_M16A4_Wrapper_C") == 0) {
            name = "M16A4";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_SCAR_Wrapper_C") == 0) {
            name = "SCAR";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_QBZ_Wrapper_C") == 0) {
            name = "QBZ";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_G36_Wrapper_C") == 0) {
            name = "G36C";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_M762_Wrapper_C") == 0) {
            name = "M762";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_Groza_Wrapper_C") == 0) {
            name = "Groza";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_AUG_Wrapper_C") == 0) {
            name = "AUG";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_Dp-28_Wrapper_C") == 0) {
            name = "Dp-28";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_DP28_Wrapper_C") == 0) {
            name = "DP28";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_M249_Wrapper_C") == 0) {
            name = "M249";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_QBU_Wrapper_C") == 0) {
            name = "QBU";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_VAL_Wrapper_C") == 0) {
            name = "AV-VAL";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_MG3_Wrapper_C") == 0) {
            name = "MG3";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_HoneyBadger_Wrapper_C") == 0) {
            name = "蜜獾";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_Mk47_Wrapper_C") == 0) {
            name = "Mk47";
            *color = rifle_color;
            return name;
        }
    }
    //显示冲锋枪
    if (showSubmachine) {
        if (strcmp(gname, "BP_MachineGun_UMP9_Wrapper_C") == 0) {
            name = "UMP9";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_TommyGun_Wrapper_C") == 0) {
            name = "汤姆逊冲锋枪";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_PP19_Wrapper_C") == 0) {
            name = "PP19";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_Uzi_Wrapper_C") == 0) {
            name = "Uzi";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_Vector_Wrapper_C") == 0) {
            name = "Vector";
            *color = submachine_color;
            return name;
        }
        if (strstr(gname, "P90") != nullptr) {
            name = "P90";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_MP5K_Wrapper_C") == 0) {
            name = "MP5K";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_VSS_Wrapper_C") == 0) {
            name = "VSS";
            *color = submachine_color;
            return name;
        }
    }
    //显示狙击枪
    if (showSniper) {

        if (strcmp(gname, "BP_Sniper_Kar98k_Wrapper_C") == 0) {
            name = "Kar98k";
            *color = snipe_color;
            return name;
        }
        if (strstr(gname, "Mosin") != nullptr) {
            name = "莫辛纳甘";
            *color = snipe_color;
            return name;
        }

        if (strcmp(gname, "BP_Sniper_Mini14_Wrapper_C") == 0) {
            name = "Mini14";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_SKS_Wrapper_C") == 0) {
            name = "SKS";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_M24_Wrapper_C") == 0) {
            name = "M24";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_rifle_Mk47_Wrapper_C") == 0) {
            name = "Mk47";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_WEP_Mk14_Pickup_C") == 0) {
            name = "MK14";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_AWM_Wrapper_C") == 0) {
            name = "AWM";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_SLR_Wrapper_C") == 0) {
            name = "SLR";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_CrossbowBorderland_Wrapper_C") == 0) {
            name = "战术弩";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_MK12_Wrapper_C") == 0) {
            name = "MK12";
            *color = snipe_color;
            return name;
        }
        if (strstr(gname, "AMR") != nullptr) {
            name = "AMR";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_Win94_Wrapper_C") == 0) {
            name = "Win94";
            *color = snipe_color;
            return name;
        }
    }
    //倍镜
    if (showMirror) {
        if (strstr(gname, "MZJ_8X") != nullptr) {
            name = "8倍镜";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_6X") != nullptr) {
            name = "6倍镜";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_4X") != nullptr) {
            name = "4倍镜";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_3X") != nullptr) {
            name = "3倍镜";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_2X") != nullptr) {
            name = "2倍镜";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_HD") != nullptr) {
            name = "红点瞄准镜";
            *color = otherparts_color;
            return name;
        }
        if (strstr(gname, "MZJ_QX") != nullptr) {
            name = "全息瞄准镜";
            *color = otherparts_color;
            return name;
        }
    }
    //配件
    if (showExpansion) {
        if (strcmp(gname, "BP_DJ_Large_EQ_Pickup_C") == 0) {
            name = "步枪快扩";
            *color = expansion_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Large_E_Pickup_C") == 0) {
            name = "步枪扩容";
            *color = expansion_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Sniper_EQ_Pickup_C") == 0) {
            name = "狙击枪快扩";
            *color = mirror_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Sniper_E_Pickup_C") == 0) {
            name = "狙击枪扩容";
            *color = mirror_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Mid_E_Pickup_C") == 0) {
            name = "冲锋枪扩容";
            *color = mirror_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Mid_EQ_Pickup_C") == 0) {
            name = "冲锋枪快扩";
            *color = mirror_color;
            return name;
        }
    }
    //其他配件
    if (showOtherParts) {
        if (strcmp(gname, "BP_QK_Large_Suppressor_Pickup_C") == 0) {
            name = "步枪消音";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Sniper_Suppressor_Pickup_C") == 0) {
            name = "狙击枪消音";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QT_Sniper_Pickup_C") == 0) {
            name = "狙击枪托腮板";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_ZDD_Sniper_Pickup_C") == 0) {
            name = "狙击枪子弹袋";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Large_Compensator_Pickup_C") == 0) {
            name = "步枪枪口补偿";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Sniper_Compensator_Pickup_C") == 0) {
            name = "狙击枪补偿器";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_Vertical_Pickup_C") == 0) {
            name = "步槍垂直握把";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QT_A_Pickup_C") == 0) {
            name = "步枪枪托";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_Angled_Pickup_C") == 0) {
            name = "步枪直角握把";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_ThumbGrip_Pickup_C") == 0) {
            name = "步枪拇指握把";
            *color = otherparts_color;
            return name;
        }
        if (strstr(gname, "Ghillie")) {
            name = "吉利服";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_DuckBill_Pickup_C") == 0) {
            name = "鸭嘴收缩器";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Choke_Pickup_C") == 0) {
            name = "收缩器";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_ZDD_Crossbow_Q_Pickup_C") == 0) {
            name = "箭袋";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_ShotGun_Pickup_C") == 0) {
            name = "霰弹快速装填";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_LightGrip_Pickup_C") == 0) {
            name = "轻型握把";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_HalfGrip_Pickup_C") == 0) {
            name = "半截式握把";
            *color = otherparts_color;
            return name;
        }if (strcmp(gname, "BP_WB_Lasersight_Pickup_C") == 0) {
            name = "激光瞄准器";
            *color = otherparts_color;
            return name;
        }
    }
    //药品显示
    if (showDrug) {
        if (strstr(gname, "Injection")) {
            name = "肾上腺素";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Firstaid")) {
            name = "急救包";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "FirstAidbox")) {
            name = "医疗箱";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Pills")) {
            name = "止痛药";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Drink")) {
            name = "能量饮料";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Bandage")) {
            name = "止血绷带";
            *color = drug_color;
            return name;
        }
    }
    //甲和头盔
    if (showArmor) {
        if (strstr(gname, "Helmet_Lv3")) {
            name = "三级头";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Armor_Lv3")) {
            name = "三级甲";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Bag_Lv3")) {
            name = "三级包";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Helmet_Lv2")) {
            name = "二级头";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Armor_Lv2")) {
            name = "二级甲";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Bag_Lv2")) {
            name = "二级包";
            *color = armor_color;
            return name;
        }
        /*if (strstr(gname, "Helmet_Lv1")) {
            name = "一级头";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Armor_Lv1")) {
            name = "一级甲";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Bag_Lv1")) {
            name = "一级包";
            *color = armor_color;
            return name;
        }*/
    }

    //特殊模式工具
    if (showSpecial) {
        if (strstri(gname, "AssaultSoldier")) {
            name = "突击兵_特种装备";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "SupplySoldier")) {
            name = "后勤兵_特种装备";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "EngineerSoldier")) {
            name = "工程兵_特种装备";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "AgileSoldier")) {
            name = "机动兵_特种装备";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "MedicSoldier")) {
            name = "医疗兵_特种装备";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "CoreRou")) {
            name = "圆形升级核心";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "CoreTri")) {
            name = "三角升级核心";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "CoreSqu")) {
            name = "方形升级核心";
            *color = special_color;
            return name;
        }

        if (strstri(gname, "waiguge")) {
            name = "外骨骼蓝图";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "SparLv1")) {
            name = "纳米晶体";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "ResurrectionCard")) {
            name = "召回信标";
            *color = special_color;
            return name;
        }

    }

    if (showOther) {
        if (strstri(gname, "GasCanBattery_Destructible")) {
            name = "油桶";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "EntranceFactoryCard")) {
            name = "3周年工卡";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "FlareGun")) {
            name = "信号枪";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "BP_revivalAED_Pickup_C")) {
            name = "自救器";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "HumanCannon_Pickup_C")) {
            name = "弹射炮";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "PickUp_BP_Partner")) {
            name = "伙伴";
            *color = other_color;
            return name;
        }
    }

    return "";
}

//获取载具
string GetVehicleInfo(const char *gname, ImVec4 *color) {
    string name;
    if (strcmp(gname, "VH_BRDM_C") == 0) {
        name = "装甲车";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_Scooter_C") == 0) {
        name = "电瓶车";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_Motorcycle_C") == 0 || strcmp(gname, "VH_Motorcycle_1_C") == 0) {
        name = "摩托車";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_MotorcycleCart_1_C") == 0 || strcmp(gname, "VH_MotorcycleCart_C") == 0) {
        name = "三轮车";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_Snowmobile_C") == 0) {
        name = "雪地摩托";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "BP_VH_Tuk_C") == 0 || strcmp(gname, "BP_VH_Tuk_1_C") == 0) {
        name = "三轮摩托";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Buggy")) {
        name = "蹦蹦";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Dacia")) {
        name = "小轿车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "UAZ")) {
        name = "吉普车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "_PickUp")) {
        name = "皮卡车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Rony")) {
        name = "皮卡车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Mirado")) {
        name = "跑车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "MiniBus")) {
        name = "宝宝巴士";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "PG117")) {
        name = "快艇";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "AquaRail")) {
        name = "摩托艇";
        *color = vehicle_color;
        return name;
    }

    if (strstri(gname, "AquaRail")) {
        name = "摩托艇";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "AirDropPlane")) {
        name = "放置飞机";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "BP_AirDropPlane_C")) {
        name = "电动滑翔机";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Bigfoot")) {
        name = "大脚车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "ATV")) {
        name = "沙滩车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "CoupeRB")) {
        name = "跑车";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "BP_SciFi_Moto_C")) {
        name = "啵啵车";
        *color = vehicle_color;
        return name;
    }

    return "";
}

//获取手持
string getWeapon(int id) {
    if (id == 101006) {
        return "AUG";
    } else if (id == 101008) {
        return "M762";
    } else if (id == 101003) {
        return "SCAR-L";
    } else if (id == 101004) {
        return "M416";
    } else if (id == 101004) {
        return "M16A-4";
    } else if (id == 101009) {
        return "Mk47";
    } else if (id == 101010) {
        return "G36C";
    } else if (id == 101007) {
        return "QBZ";
    } else if (id == 101001) {
        return "AKM";
    } else if (id == 101005) {
        return "Groza";
    } else if (id == 102005) {
        return "野牛";
    } else if (id == 102004) {
        return "汤姆逊";
    } else if (id == 102007) {
        return "MP5K";
    } else if (id == 102002) {
        return "UMP45";
    } else if (id == 102003) {
        return "Vector";
    } else if (id == 102001) {
        return "Uzi";
    } else if (id == 105002) {
        return "DP28";
    } else if (id == 105001) {
        return "M249";
    } else if (id == 103001) {
        return "Kar98k";
    } else if (id == 103002) {
        return "M24";
    } else if (id == 103003) {
        return "AWM";
    } else if (id == 103010) {
        return "QBU";
    } else if (id == 103009) {
        return "SLR";
    } else if (id == 103004) {
        return "SKS";
    } else if (id == 103006) {
        return "Mini14";
    } else if (id == 103005) {
        return "VSS";
    } else if (id == 103008) {
        return "Win94";
    } else if (id == 103007) {
        return "Mk14";
    } else if (id == 103903) {
        return "莫辛甘纳";
    } else if (id == 104003) {
        return "S12K";
    } else if (id == 104004) {
        return "DBS";
    } else if (id == 104001) {
        return "S686";
    } else if (id == 104002) {
        return "S1897";
    } else if (id == 108003) {
        return "镰刀";
    } else if (id == 108001) {
        return "大砍刀";
    } else if (id == 108002) {
        return "撬棍";
    } else if (id == 107001) {
        return "十字弩";
    } else if (id == 108004) {
        return "平底锅";
    } else if (id == 106006) {
        return "短管霰弹";
    } else if (id == 106003) {
        return "R1895";
    } else if (id == 106008) {
        return "Vz61";
    } else if (id == 106001) {
        return "P92";
    } else if (id == 106004) {
        return "P18C";
    } else if (id == 106005) {
        return "R45";
    } else if (id == 106002) {
        return "P1911";
    } else if (id == 106010) {
        return "沙漠之鹰";
    } else if (id == 106107) {
        return "乱斗手枪";
    } else if (id == 101002) {
        return "M16A4";
    } else if (id == 101011) {
        return "AC-VAL";
    } else if (id == 102105) {
        return "P90";
    } else if (id == 103012) {
        return "AMR";
    } else if (id == 103013) {
        return "M417";
    } else if (id == 103100) {
        return "Mk12";
    } else if (id == 103901) {
        return "Kar98K";
    } else if (id == 103902) {
        return "M24";
    } else if (id == 103903) {
        return "AWM";
    } else if (id == 103903) {
        return "SPAS-12";
    } else if (id == 105002) {
        return "DP-28";
    } else if (id == 105003) {
        return "M134";
    } else if (id == 105010) {
        return "MG3";
    } else if (id == 107986) {
        return "Kar98K";
    } else if (id == 602001) {
        return "震爆弹";
    } else if (id == 602002) {
        return "烟雾弹";
    } else if (id == 602003) {
        return "燃烧瓶";
    } else if (id == 602004) {
        return "破片手榴弹";
    } else if (id == 0) {
        return "拳头";
    } else if (id == 101012) {
        return "蜜獾";
    } else {
        return "未知("+ to_string(id) +")";
    }
}