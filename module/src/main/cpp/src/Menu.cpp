#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-integer-division"
#pragma ide diagnostic ignored "EndlessLoop"
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "readability-redundant-declaration"
#pragma ide diagnostic ignored  "ConstantFunctionResult"
#pragma ide diagnostic ignored "-Wunknown-pragmas"

#include <jni.h>
#include <string>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <string>

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <dlfcn.h>
#include <android/configuration.h>
#include <android/native_activity.h>
#include <sys/system_properties.h>
#include <syscall.h>
#include <sys/prctl.h>

#include <MemoryTools.h>
#include <MapTools.h>
#include "GameUtils.h"
#include "HttpUtils.h"
#include "RC4Utils.h"
#include <RecorderTools.h>
#include <Logger.h>
#include "imgui.h"
#include "imgui_internal.h"
#include <backends/imgui_impl_android.h>
#include <sys/stat.h>
#include "Menu.h"
#include "JniHelper.h"
#include "ConfigUtils.h"
#include <MainLooper.h>
#include <backends/imgui_impl_opengl3.h>
#include <elf_util.h>
#include <SubstrateHook.h>
#include <files.h>
#include <regex>
#include <base64/base64.h>
#include <imgui_expand.h>
#include <hide_utils.h>
#include <bytehook.h>

using namespace std;

/*************************** 绘制配置 ***********************************/
bool showHealth = true, showLine = true, showCrosshair = true, showDis = true, showInfo = true, showRadar = false, showName = true, showTeam = true, showWeapon = true;
bool showBone = true, showBox = true;
bool showItem = true, show556 = true, showRifle = true, showMirror = true, showExpansion = true, showDrug = true, showArmor = true, showSubmachine = true, showSniper = true, showOtherParts = true, showSpecial = true, showOther = true;
bool showVehicle = true, showGrenade = true, isVisibility = true;
bool showChest = true, showDrop = true;
bool showTouzhi = true, showSandan = true;
bool aimShortDis = false;
bool showRange = true;
bool showTarget = true;
float radarLocation = 100.f;
float boneWidth = 1, boxWidth = 1, lineWidth = 1;
bool aimFire = true, aimOpen = false;

ImVec4 bone_color = Color.White, box_color = Color.White, line_color = Color.White, fallen_color = Color.Black_, visibility_color = Color.Red, ai_color = Color.Green_, info_color = Color.White;

ImVec4 radar_color = ImVec4(0 / 255.f, 255 / 255.f, 255 / 255.f, 117 / 255.f);//NOLINT
ImVec4 drop_color = ImVec4(255 / 255.f, 0 / 255.f, 0 / 255.f, 255.f / 255.f);//NOLINT
ImVec4 aim_color = ImVec4(255 / 255.f, 0 / 255.f, 0 / 255.f, 255 / 255.f);//NOLINT
//药品
ImVec4 drug_color = ImVec4(255 / 255.f, 105 / 255.f, 180 / 255.f, 255.f / 255.f);//NOLINT
//背敌预警颜色
ImVec4 warning_color = ImVec4(255 / 255.f, 255 / 255.f, 0 / 255.f, 200.f / 255.f);//NOLINT
//载具颜色
ImVec4 vehicle_color = ImVec4(255.f / 255.f, 255 / 255.f, 0 / 255.f, 255 / 255.f);//NOLINT
//装备颜色
ImVec4 armor_color = ImVec4(255 / 255.f, 215 / 255.f, 0 / 255.f, 255.f / 255.f);//NOLINT
//狙击枪颜色
ImVec4 snipe_color = ImVec4(144 / 255.f, 238 / 255.f, 0 / 255.f, 255.f / 255.f);//NOLINT
//子弹颜色
ImVec4 color_bullet = ImVec4(255 / 255.f, 255 / 255.f, 0 / 255.f, 255.f / 255.f);//NOLINT
//冲锋枪颜色
ImVec4 submachine_color = ImVec4(255 / 255.f, 110 / 255.f, 180 / 255.f, 255.f / 255.f);//NOLINT
//投掷物颜色
ImVec4 missile_color = ImVec4(255 / 255.f, 0 / 255.f, 0 / 255.f, 255.f / 255.f);//NOLINT
//步枪
ImVec4 rifle_color = ImVec4(255 / 255.f, 0 / 255.f, 0 / 255.f, 255.f / 255.f);//NOLINT
//散弹枪颜色
ImVec4 color_shot = ImVec4(255 / 255.f, 255 / 255.f, 0 / 255.f, 255.f / 255.f);//NOLINT
//倍镜颜色
ImVec4 mirror_color = ImVec4{255.f / 255.f, 144.f / 255.f, 0.f / 255.f, 255.f / 255.f};//NOLINT
//配件颜色
ImVec4 expansion_color = ImVec4{208.f / 255.f, 128.f / 255.f, 114.f / 255.f, 255.f / 255.f};//NOLINT
//其他配件颜色
ImVec4 otherparts_color = ImVec4{245.f / 255.f, 245.f / 255.f, 245.f / 255.f,
                                 255.f / 255.f};//NOLINT
//其他颜色
ImVec4 other_color = ImVec4{245.f / 255.f, 245.f / 255.f, 245.f / 255.f, 255.f / 255.f};//NOLINT
//特殊模式颜色
ImVec4 special_color = ImVec4{245.f / 255.f, 245.f / 255.f, 245.f / 255.f, 255.f / 255.f};//NOLINT
//雷达
bool showRadarDis = true;
bool showRadarBox = true;
int radarType = 1;
bool showNum = true;
bool visibilityAim = false, dieNoAim = false, aiAim = true, isDlip = false, aimbot = false, my_aimbot = false, showBoxItem = true;
int aimLocation = 3, aimedMode = 3, aimChoose = 1, aimType = 0;
float openAimRange = 150.f, closureAimRange = 150.f, touchSpeed = 50.f, touchPress = -10, aimPress = 1.0f, aimSpeed = 10.0f, aimAnticipation = 10.0f;
float max_shake = 0.3;
float SlideRanges = 200;
float draw_scale = 1.5f;
//烟雾不瞄
float projSomoke = 200;
bool isProjSomoke = true, isProjSomokeRange = false;
bool showEagleWatch = true;
Vector2A radarOffset = {0, 0};//NOLINT

int radarOffsetX = 400;
int radarOffsetY = 300;
float radarSize = 100;

float VehicleDis = 500;
float ItemDis = 20;
float BoxDis = 150;
float AirDis = 600;
int Fps = 90;

//悬浮图标
TextureInfo imageButton;
ImVec2 suspensionAimPos;//NOLINT
ImVec2 suspensionPos;//NOLINT
uintptr_t libUE4 = 0;
int glWidth = 0, glHeight = 0;
int screenWidth = -1, screenHeight = -1;

bool isPassMincore = false;
bool isAimSwichObject = true;

static EGLDisplay g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface g_EglSurface = EGL_NO_SURFACE;
static EGLContext g_EglContext = EGL_NO_CONTEXT;
static ANativeWindow *g_NativeWindow = nullptr;

static bool getBootMode() {
    // 上锁避免多线程问题
    lock(syscall_thread_ptr);
    uint64_t args[7] = {0};
    unsigned char vector[1] = {0};
    args[0] = __NR_mincore;
    args[1] = (uint64_t)ptr;
    args[2] = PAGE_SIZE;
    args[3] = (uint64_t)vector;
    call_task(syscall_thread_ptr, args, 0);
    // 解锁
    unlock(syscall_thread_ptr);
    return vector[0] == 0;
}


static void *thread_showDialog(void *arg) {
    prctl(PR_SET_NAME, "showDialog");
    auto *msg = (Message *) arg;
    if (msg->what == 1) {
        showMessage("请给予游戏文件存储权限后再使用!");
    } else {
        showMessage("请给予游戏悬浮窗权限后再使用!");
    }
    sleep(5);
    isShowDialog = false;
    return nullptr;
}


void (*orig_initializeMotionEvent)(void *a1, void *a2, void *a3);

/*void initializeMotionEvent(void *a1, void *a2, void *a3) {
    SHADOWHOOK_STACK_SCOPE();
    SHADOWHOOK_CALL_PREV(initializeMotionEvent, a1, a2, a3);
    orig_initializeMotionEvent(a1, a2, a3);
    if (g_Initialized) {
        ImGui_ImplAndroid_HandleInputEvent((AInputEvent *) a1);
    }
}
*/
void initializeMotionEvent(void *thiz, void *a2, void *a3) {
    BYTEHOOK_STACK_SCOPE( );
    BYTEHOOK_CALL_PREV(initializeMotionEvent, thiz, a2, a3);
    if (thiz != nullptr && g_Initialized) {
        ImGui_ImplAndroid_HandleInputEvent((AInputEvent *) thiz);
    }
    BYTEHOOK_POP_STACK( );
}

void initNativeDraw() {
    if (g_Initialized)
        return;
    DisplayMetrics ScreenInfo{0, 0, 0};
    while (ScreenInfo.heightPixels < 8) {
        ScreenInfo = getGameScreenInfo();
        sleep(1);
    }
    ANativeWindow_acquire(g_NativeWindow);
    {
        g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(g_EglDisplay, nullptr, nullptr);
        const EGLint egl_attributes[] = {EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
                                         EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE,
                                         EGL_WINDOW_BIT, EGL_NONE};
        EGLint num_configs = 0;
        eglChooseConfig(g_EglDisplay, egl_attributes, nullptr, 0, &num_configs);
        EGLConfig egl_config;
        eglChooseConfig(g_EglDisplay, egl_attributes, &egl_config, 1, &num_configs);
        EGLint egl_format;
        eglGetConfigAttrib(g_EglDisplay, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
        ANativeWindow_setBuffersGeometry(g_NativeWindow, ScreenInfo.widthPixels,ScreenInfo.heightPixels, egl_format);//缩放
        const EGLint egl_context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        g_EglContext = eglCreateContext(g_EglDisplay, egl_config, EGL_NO_CONTEXT,egl_context_attributes);
        g_EglSurface = eglCreateWindowSurface(g_EglDisplay, egl_config, g_NativeWindow, nullptr);
        eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    //初始化绘制纹理
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui_ImplAndroid_Init(nullptr, {(float) screenWidth / (float) ScreenInfo.widthPixels,(float) screenHeight / (float) ScreenInfo.heightPixels});

    float density = AConfiguration_getDensity(g_App->config);

    ImGuiStyle &style = ImGui::GetStyle();

    //窗口圆角
    style.ScrollbarRounding = 0;//滚动条抓取角的半径
    style.ScrollbarSize /= 2;//垂直滚动条的宽度，水平滚动条的高度
    style.TabRounding = 0; //圆角
    style.ScaleAllSizes(std::max(1.0f, density / 140.0f));

    size_t length = (files::logoDataBase64.length() + 1) / 4 * 3;
    unsigned char *data = base64_decode((unsigned char *) files::logoDataBase64.c_str());
    imageButton = ImGui::loadTextureFromMemory(data, length);
    free(data);


    InitTexture();

    if (markcode.empty()) {
        markcode = getMacAddress();
    }

    loginInfo = "请验证账号";

    g_Initialized = true;
}

void DestroyNativeDraw() {
    if (!g_Initialized)
        return;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (g_EglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (g_EglContext != EGL_NO_CONTEXT)
            eglDestroyContext(g_EglDisplay, g_EglContext);

        if (g_EglSurface != EGL_NO_SURFACE)
            eglDestroySurface(g_EglDisplay, g_EglSurface);

        eglTerminate(g_EglDisplay);
    }

    g_EglDisplay = EGL_NO_DISPLAY;
    g_EglContext = EGL_NO_CONTEXT;
    g_EglSurface = EGL_NO_SURFACE;
    ANativeWindow_release(g_NativeWindow);

    g_Initialized = false;
}

void DrawData(bool type) {
    if (type && g_EglDisplay == EGL_NO_DISPLAY)
        return;
    if (type) {
        eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_WIDTH, &glWidth);
        eglQuerySurface(g_EglDisplay, g_EglSurface, EGL_HEIGHT, &glHeight);
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(glWidth, glHeight);
    ImGui::NewFrame();

    // 是否自瞄
    aimbot = my_aimbot;

    //登录弹窗
    if (!isLogin) {
        if (ImGui::Begin("账号登录", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
            // 帐号输入框背景为透明
            my_window_focused = ImGui::IsWindowFocused();
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0, 0.0, 0.0, 0.0));
            //窗口字体缩放
            ImGui::SetWindowFontScale(1.5f);
            //帐号窗口居中
            ImGui::SetWindowPos(ImVec2(glWidth / 2 - ImGui::GetWindowWidth() / 2,glHeight / 2 - ImGui::GetWindowHeight() / 2));
            char password[64] = {0};
            my_strcpy(password, InputPwd.c_str());
            ImGui::Text("%s", loginInfo.c_str());
            ImGui::InputTextWithHint("##账号", "输入账号", password, IM_ARRAYSIZE(password));
            ImGui::SameLine();
            if (ImGui::Button("登录")) {
                loginInfo = "正在验证中...";
                pthread_t t1;
                pthread_create(&t1, nullptr, LoginUin, nullptr);
            }

            if (io.WantTextInput && !WantTextInputLast) {
                ShowSoftKeyboardInput(524289, "这个没用这个参数", InputPwd.c_str());
                ImGui::FocusWindow(nullptr);
            }

            WantTextInputLast = io.WantTextInput;
            if (bKeyboardShowing()) {
                InputPwd = GetSoftKeyboardInput();
            }

            if (isAutoLogin && !isTestLogin) {
                isTestLogin = true;
                loginInfo = "正在验证中...";
                pthread_t t1;
                pthread_create(&t1, nullptr, LoginUin, nullptr);
            }
            ImGui::PopStyleColor(1);

        }
        ImGui::End();
    }

    //功能窗口
    if (isLogin) {
        
        //更新数据  线程
        if (isKernel && KernelState) {
            if (!initUpdateList) {
                pthread_t update_list;
                pthread_create(&update_list, nullptr, updateDataList, nullptr);
                initUpdateList = true;
            }
            createDataList();
        }

        //悬浮球
        if (!my_window_open) {
            if (ImGui::Begin("##悬浮球", nullptr,ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {

                if (!isSetWindowPos) {
                    if (SaveWindowPos.x != -520 && SaveWindowPos.y != -520)
                        ImGui::SetWindowPos(SaveWindowPos);
                    isSetWindowPos = true;
                }

                //是否按住编辑等
                my_window_focused = ImGui::IsWindowFocused();
                if (ImGui::IsItemActive()) {
                    if (!isImageDown) {
                        //按下
                        isImageDown = true;
                        suspensionPos = ImGui::GetWindowPos();
                    }
                } else if (isImageDown) {
                    //未按下
                    isImageDown = false;
                    if (suspensionPos.x == ImGui::GetWindowPos().x && ImGui::GetWindowPos().y == suspensionPos.y)
                        my_window_open = !my_window_open;
                }

                static float LogoSize = 75;//dip2px(40);

                ImGui::Image(imageButton.textureId, ImVec2{LogoSize, LogoSize}, ImVec2{0, 0},ImVec2{1, 1});

                if (isSetWindowPos)
                    SaveWindowPos = ImGui::GetWindowPos();
            }
            ImGui::End();
        }

        //菜单
        if (my_window_open) {
            ImGui::SetNextWindowSize(ImVec2(glWidth * 0.45f, glHeight * 0.70f),ImGuiCond_Once); // 45% width 70% height
            if (ImGui::Begin(main_window_title, &my_window_open,ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse)) {
                my_window_focused = ImGui::IsWindowFocused();
                if (ImGui::BeginTabBar("Tab", 1)) {
                    if (ImGui::BeginTabItem("信息")) {
                        ImGui::Spacing();
                        ImGui::TextDisabled("系统公告%s", NoteDate.c_str());
                        ImGui::TextWrapped("%s", "暂无公告" );
                        ImGui::TextDisabled("更新内容");
                        ImGui::TextWrapped("%s", "获取失败,但是不影响使用" );
                        ImGui::TextDisabled("系统信息");
                        ImGui::TextWrapped("检测状态: [%s]", isPassMincore ? "过检测成功" : "过检测失败");
                        ImGui::TextWrapped("在线人数: [%d]", onelineNum);
                        ImGui::TextWrapped("用户账号: %s [%s]", UserName.c_str(), InputPwd.c_str());
                        ImGui::TextWrapped("绘制帧率: [%.3f Ms / %.1f Fps]", 1000.0f / io.Framerate, io.Framerate);
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("透视")) {
                        ImGui::Spacing();
                        if (ImGui::BeginTable("split", 2)) {
                            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示方框", &showBox);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示骨骼", &showBone);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示血量", &showHealth);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示射线", &showLine);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示距离", &showDis);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示队编", &showTeam);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("人机判断", &showInfo);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示昵称", &showName);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示准心", &showCrosshair);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示雷达", &showRadar);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("预警提示", &showGrenade);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("掩体判断", &isVisibility);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("手持武器", &showWeapon);
                            if (strcmp(Version.c_str(), "3.7.1") == 0) {
                                ImGui::TableNextColumn();
                                ImGui::Checkbox("猎鹰提示", &showEagleWatch);
                            }

                            ImGui::TableNextColumn();
                            ImGui::Checkbox("人数显示", &showNum);
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("物资")) {
                        ImGui::Spacing();
                        ImGui::Checkbox("载具", &showVehicle);
                        ImGui::SameLine();
                        ImGui::Checkbox("物品", &showItem);
                        ImGui::SameLine();
                        ImGui::Checkbox("盒子", &showChest);
                        ImGui::SameLine();
                        ImGui::Checkbox("空投", &showDrop);
                        ImGui::SameLine();
                        ImGui::Checkbox("盒内物资", &showBoxItem);

                        if (ImGui::BeginTable("split", 2)) {
                            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示步枪", &showRifle);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示冲锋枪", &showSubmachine);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示狙击枪", &showSniper);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示子弹", &show556);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示倍镜", &showMirror);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示快扩", &showExpansion);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("其他配件", &showOtherParts);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示药品", &showDrug);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示防具", &showArmor);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示投掷物", &showTouzhi);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("显示散弹枪", &showSandan);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("特殊物资", &showSpecial);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("其他物资", &showOther);
                            ImGui::EndTable();
                        }

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("自瞄")) {
                        ImGui::Spacing();

                        if (ImGui::BeginTable("split", 2)) {
                            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("开启自瞄", &my_aimbot);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("掩体自瞄", &visibilityAim);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("人机瞄准", &aiAim);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("倒地瞄准", &dieNoAim);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("烟雾不瞄", &isProjSomoke);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("烟雾方块", &isProjSomokeRange);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("自瞄圈圈", &showRange);
                            ImGui::TableNextColumn();
                            ImGui::Checkbox("强制锁定", &isAimSwichObject);
                            ImGui::TableNextColumn();
                            ImGui::SliderFloat("自瞄横扫", &max_shake, 0.0f, 1.5f);
                            ImGui::TableNextColumn();
                            ImGui::SliderFloat("烟雾大小", &projSomoke, 15.0f, 1200.0f);
                            ImGui::TableNextColumn();
                            if (openAimRange > 1000.f) {
                                openAimRange = 1000.f;
                            }
                            ImGui::SliderFloat("开镜范围", &openAimRange, 0.0f, 1000.0f);
                            ImGui::TableNextColumn();
                            if (closureAimRange > 1000.f) {
                                closureAimRange = 1000.f;
                            }
                            ImGui::SliderFloat("腰射范围", &closureAimRange, 0.0f, 1000.0f);
                            ImGui::TableNextColumn();
                            ImGui::SliderFloat("自瞄压枪", &aimPress, 0.0f, 2.0f);
                            ImGui::TableNextColumn();
                            ImGui::SliderFloat("自瞄速度", &aimSpeed, 0.9f, 60.0f);
                            ImGui::TableNextColumn();
                            ImGui::SliderFloat("自瞄预判", &aimAnticipation, 0.0f, 60.0f);
                            ImGui::TableNextColumn();
                            ImGui::TableNextColumn();
                            if (ImGui::BeginTable("split", 1)) {
                                ImGui::TableNextColumn();
                                const char *aimingState[] = {"开镜自瞄", "开火自瞄", "全部自瞄", "自动模式"};
                                ImGui::Combo("自瞄模式", &aimedMode, aimingState,
                                             IM_ARRAYSIZE(aimingState));
                                ImGui::EndTable();
                            }
                            ImGui::TableNextColumn();
                            if (ImGui::BeginTable("split", 1)) {
                                ImGui::TableNextColumn();
                                const char *Choose[] = {"屏幕中心", "自动选择", "距离优先"};
                                ImGui::Combo("自瞄选择", &aimChoose, Choose, IM_ARRAYSIZE(Choose));
                                ImGui::EndTable();
                            }
                            ImGui::TableNextColumn();
                            if (ImGui::BeginTable("split", 1)) {
                                ImGui::TableNextColumn();
                                const char *items[] = {"头部", "胸部", "腰部", "掩体外"};
                                ImGui::Combo("自瞄位置", &aimLocation, items, IM_ARRAYSIZE(items));
                                ImGui::EndTable();
                            }
                            ImGui::TableNextColumn();
                            if (ImGui::BeginTable("split", 1)) {
                                ImGui::TableNextColumn();
                                const char *items[] = {"函数自瞄", "子弹追踪[危险]"};
                                ImGui::Combo("自瞄类型", &aimType, items, IM_ARRAYSIZE(items));
                                ImGui::EndTable();
                            }
                            ImGui::TableNextColumn();
                            ImGui::EndTable();
                        }

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("设置")) {
                        ImGui::Spacing();

                        if (ImGui::CollapsingHeader("雷达设置")) {
                            ImGui::Checkbox("显示距离", &showRadarDis);
                            ImGui::SameLine();
                            ImGui::Checkbox("显示外框", &showRadarBox);
                            ImGui::SameLine();
                            ImGui::RadioButton("小地图雷达", &radarType, 1);
                            ImGui::SameLine();
                            ImGui::RadioButton("360°圆形雷达", &radarType, 2);
                            if (radarType == 1) {
                                ImGui::SliderFloat("雷达大小", &radarSize, 80.0f, 800.0f);
                            }
                            ImGui::SliderInt("雷达横轴", &radarOffsetX, -1000, 3000);
                            ImGui::SliderInt("雷达纵轴", &radarOffsetY, -1000, 3000);
                        }


                        if (ImGui::CollapsingHeader("线条粗细")) {
                            ImGui::SliderFloat("方框粗细", &boxWidth, 1, 15);
                            ImGui::SliderFloat("骨骼粗细", &boneWidth, 1, 15);
                            ImGui::SliderFloat("射线粗细", &lineWidth, 1, 15);
                        }

                        if (ImGui::CollapsingHeader("颜色设置")) {
                            ImGui::ColorEdit4("载具颜色", (float *) &vehicle_color);
                            ImGui::ColorEdit4("步枪颜色", (float *) &rifle_color);
                            ImGui::ColorEdit4("冲锋枪颜色", (float *) &submachine_color);
                            ImGui::ColorEdit4("狙击枪颜色", (float *) &snipe_color);
                            ImGui::ColorEdit4("倍镜颜色", (float *) &mirror_color);
                            ImGui::ColorEdit4("配件颜色", (float *) &expansion_color);
                            ImGui::ColorEdit4("其他配件颜色", (float *) &otherparts_color);
                            ImGui::ColorEdit4("其他物资颜色", (float *) &other_color);
                            ImGui::ColorEdit4("特殊物资颜色", (float *) &special_color);
                            ImGui::ColorEdit4("药品颜色", (float *) &drug_color);
                            ImGui::ColorEdit4("防具颜色", (float *) &armor_color);
                            ImGui::ColorEdit4("投掷物颜色", (float *) &missile_color);
                            ImGui::ColorEdit4("散弹枪颜色", (float *) &color_shot);
                            ImGui::ColorEdit4("子弹颜色", (float *) &color_bullet);
                            ImGui::ColorEdit4("射线颜色", (float *) &line_color);
                            ImGui::ColorEdit4("方框颜色", (float *) &box_color);
                            ImGui::ColorEdit4("骨骼顔色", (float *) &bone_color);
                            ImGui::ColorEdit4("倒地颜色", (float *) &fallen_color);
                            ImGui::ColorEdit4("人机颜色", (float *) &ai_color);
                            ImGui::ColorEdit4("掩体颜色", (float *) &visibility_color);
                            ImGui::ColorEdit4("文字颜色", (float *) &info_color);
                            ImGui::ColorEdit4("雷达颜色", (float *) &radar_color);
                        }

                        if (ImGui::CollapsingHeader("绘制距离")) {
                            ImGui::SliderFloat("载具距离", &VehicleDis, 0, 1500);
                            ImGui::SliderFloat("物资距离", &ItemDis, 0, 1500);
                            ImGui::SliderFloat("盒子距离", &BoxDis, 0, 1500);
                            ImGui::SliderFloat("空投距离", &AirDis, 0, 1500);
                        }

                        if (ImGui::CollapsingHeader("配置信息")) {
                            
                        }

                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            ImGui::End();
        }
    }

    //渲染
    ImGui::Render();
    if (type) {
        glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (type) eglSwapBuffers(g_EglDisplay, g_EglSurface);
}

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

EGLBoolean new_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    //获取游戏分辨率
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);

    //游戏分辨率
    if (glWidth <= 0 || glHeight <= 0)
        return orig_eglSwapBuffers(dpy, surface);

    if (!g_Initialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        //初始化绘制纹理
        ImGui_ImplOpenGL3_Init("#version 300 es");

        DisplayMetrics ScreenInfo = getScreenInfo();
        screenWidth = ScreenInfo.widthPixels;
        screenHeight = ScreenInfo.heightPixels;

        ImGui_ImplAndroid_Init(nullptr, {(float) screenWidth / (float) glWidth,(float) screenHeight / (float) glHeight});

        int density = AConfiguration_getDensity(g_App->config);

        ImGuiStyle &style = ImGui::GetStyle();

        //窗口圆角
        style.ScrollbarRounding = 0;//滚动条抓取角的半径
        style.ScrollbarSize /= 2;//垂直滚动条的宽度，水平滚动条的高度
        style.TabRounding = 0; //圆角
        style.ScaleAllSizes(std::max(1.f, density / 140.0f));

        size_t length = (files::logoDataBase64.length() + 1) / 4 * 3;
        unsigned char *data = base64_decode((unsigned char *) files::logoDataBase64.c_str());
        imageButton = ImGui::loadTextureFromMemory(data, length);
        free(data);

        InitTexture();

        if (markcode.empty()) {
            markcode = getMacAddress();
        }

        loginInfo = "请验证账号";

        glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        g_Initialized = true;
    }
    DrawData(false);
    return orig_eglSwapBuffers(dpy, surface);
}

void *thread_onDraw(void *args) {
    prctl(PR_SET_NAME, "thread_onDraw");
    sleep(1);
    JNIEnv *env = GetJavaEnv();
    auto View = (jobject) args;
    jobject surface;
    while (!g_NativeWindow) {
        if (isFakeRecorder) {
            //获取textrueView的surfaceTexrue
            jclass textrueViewClass = env->GetObjectClass(View);
            jmethodID getSurfaceTextureMethod = env->GetMethodID(textrueViewClass,"getSurfaceTexture","()Landroid/graphics/SurfaceTexture;");
            jobject surfaceTexture = env->CallObjectMethod(View, getSurfaceTextureMethod);
            if (surfaceTexture == nullptr) {
                env->DeleteLocalRef(textrueViewClass);
                continue;
            }
            //surface
            jclass surfaceClass = env->FindClass("android/view/Surface");
            jmethodID surfaceConstructor = env->GetMethodID(surfaceClass, "<init>","(Landroid/graphics/SurfaceTexture;)V");
            surface = env->NewObject(surfaceClass, surfaceConstructor, surfaceTexture);
            env->DeleteLocalRef(textrueViewClass);
            env->DeleteLocalRef(surfaceClass);
        } else {
            jclass surfaceViewClass = env->GetObjectClass(View);
            jmethodID getHolderMethod = env->GetMethodID(surfaceViewClass, "getHolder","()Landroid/view/SurfaceHolder;");
            jobject surfaceHolder = env->CallObjectMethod(View, getHolderMethod);
            if (surfaceHolder == nullptr) {
                env->DeleteLocalRef(surfaceViewClass);
                continue;
            }
            jclass surfaceHolderClass = env->GetObjectClass(surfaceHolder);
            jmethodID getSurfaceMethod = env->GetMethodID(surfaceHolderClass, "getSurface","()Landroid/view/Surface;");
            surface = env->CallObjectMethod(surfaceHolder, getSurfaceMethod);
            env->DeleteLocalRef(surfaceViewClass);
            env->DeleteLocalRef(surfaceHolderClass);
        }

        g_NativeWindow = ANativeWindow_fromSurface(env, surface);

        sleep(1);
    }

    initNativeDraw();

    int tmpValue, floatValue,recordFps;
    while (g_App->destroyRequested == 0) { //判断是否销毁
        //等待游戏返回前台
        while (g_App->activityState != 11) {
            usleep(10000);
        }
        sleep(1);
        while (g_App->activityState == 11) {
            DrawData(true);
            while ((tmpValue = getMatrixValue()) == floatValue) {
                if (tmpValue == -1) {
                    usleep((1000 / recordFps * 1000) + 8 * 1000);
                    break;
                }
                usleep(100);
                if (tmpValue <= 0) {
                    break;
                }
                recordFps = (int)ImGui::GetIO().Framerate;
            }
            floatValue = tmpValue;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        eglSwapBuffers(g_EglDisplay, g_EglSurface);
    }
    DestroyNativeDraw();
    return nullptr;
}

void hookEglSwapBuffers() {
    uintptr_t realEglSwapBuffersPtr = findEGLFun();

    if (!IsPtrValid((void *) realEglSwapBuffersPtr)) {
        Dl_info egl_info;
        dladdr((void *) eglSwapBuffers, &egl_info);
        auto libEGLBase = (uintptr_t) egl_info.dli_fbase;
        struct stat libEGLStat{};
        stat("/system/lib64/libEGL.so", &libEGLStat);
        long libEGLSize = libEGLStat.st_size;
        realEglSwapBuffersPtr = findEGLFunctionAddress(libEGLBase, libEGLSize);
    }

    uintptr_t eglSwapBuffersAddr = realEglSwapBuffersPtr;
    *(uintptr_t *) & orig_eglSwapBuffers = *(uintptr_t *) (eglSwapBuffersAddr);
    int old_prot = getMemPermission(eglSwapBuffersAddr);
    editMemProt(eglSwapBuffersAddr,PROT_READ | PROT_WRITE);
    *(uintptr_t *)
    eglSwapBuffersAddr = (uintptr_t) new_eglSwapBuffers;
    editMemProt(eglSwapBuffersAddr, old_prot);
}

void (*orig_finishEvent)(AInputQueue *queue, AInputEvent *event, int handled);
void new_finishEvent(AInputQueue *queue, AInputEvent *event, int handled) {
    if (g_Initialized) {
        ImGui_ImplAndroid_HandleInputEvent(event);
    }
    orig_finishEvent(queue, event, handled);
}

void *hook_read_thread(void *) {
#ifdef __LP64__

    sleep(5);

    libUE4 = getModule("libUE4.so", true);
    while (!libUE4) {
        libUE4 = getModule("libUE4.so", true);
        sleep(1);
    }

    setGameOffset(nullptr);

    while (!g_App) {
        g_App = getAndroidApp();
        sleep(1);
    }

    CurrentJavaVM = g_App->activity->vm;

    // hook 触摸
    bytehook_init(BYTEHOOK_MODE_AUTOMATIC, false);
    bytehook_hook_single("libinput.so", nullptr, "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE", (void *) initializeMotionEvent, nullptr, nullptr);

    // 判断小米12 和 K50
    char marketName[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.product.marketname", marketName);
    if (strstr(marketName, "12") || strstr(marketName, "K50")) {
        hookEglSwapBuffers();
        return nullptr;
    }

    while (!HasWindows) {
        HasWindows = canDrawOverlays();
        if (!HasWindows) {
            if (!isShowDialog) {
                pthread_t t1;
                Message msg{2, nullptr};
                pthread_create(&t1, nullptr, thread_showDialog, &msg);
                isShowDialog = true;
            }
        }
    }
    
    isLogin = true;
    isAutoLogin = true;
    MainLooper::GetInstance()->send({1, nullptr});
#endif
    return nullptr;
}

int handle_message(int fd, __attribute__((unused)) int events, __attribute__((unused)) void *data) {
    Message msg{};
    read(fd, &msg, sizeof(Message));
    if (msg.what == 1 && screenWidth <= 0) {
        DisplayMetrics ScreenInfo = getScreenInfo();
        screenWidth = ScreenInfo.widthPixels;
        screenHeight = ScreenInfo.heightPixels;
        jobject drawView = createDrawView(ScreenInfo);
        pthread_t create;
        pthread_create(&create, nullptr, thread_onDraw, drawView);
    }
    return 1;
}


void install_filter() {
    isPassMincore = true;
}

void *call_syscall(void *args){
    void **d_args = (void **)args;
    void *ret = (void *)syscall((long)d_args[0] ,d_args[1] ,d_args[2] ,d_args[3], d_args[4], d_args[5], d_args[6]);
    return ret;
}

void *pthread_syscall(void *args){
    auto *syscall_thread = (thread_syscall_t *)args;
    while(true){
        if (syscall_thread->isTask) {
            syscall_thread->ret = call_syscall(syscall_thread->args);
            syscall_thread->args = nullptr;
            syscall_thread->isReturn = 1;
            syscall_thread->isTask = 0;
        }
    }
    return nullptr;
}

thread_syscall_t *pthread_syscall_create() {
    auto *syscall_thread = (thread_syscall_t *)malloc(sizeof(thread_syscall_t));
    syscall_thread->type = 0;
    syscall_thread->isTask = 0;
    syscall_thread->args = nullptr;
    syscall_thread->ret = nullptr;
    syscall_thread->isReturn = 0;
    pthread_mutex_init(&syscall_thread->mutex, nullptr);
    pthread_t threadId;
    pthread_create(&threadId, nullptr, &pthread_syscall, (void *)syscall_thread);
    syscall_thread->thread = threadId;
    return syscall_thread;
}

int lock(thread_syscall_t *syscall_thread){
    return pthread_mutex_lock(&syscall_thread->mutex);
}

int unlock(thread_syscall_t *syscall_thread){
    return pthread_mutex_unlock(&syscall_thread->mutex);
}

void *call_task(thread_syscall_t *syscall_thread, void *args, int type){
    if(syscall_thread->isTask == 0){
        syscall_thread->args = args;
        syscall_thread->type = type;
        syscall_thread->isTask = 1;
    }
    do{
        if (syscall_thread->isReturn){
            syscall_thread->isReturn = 0;
            return syscall_thread->ret;
        }
    }while(true);
}
#pragma clang diagnostic pop