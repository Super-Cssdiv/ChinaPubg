// dear imgui: Platform Binding for Android native app
// This needs to be used along with the OpenGL 3 Renderer (imgui_impl_opengl3)

// Implemented features:
//  [X] Platform: Keyboard arrays indexed using AKEYCODE_* codes, e.g. ImGui::IsKeyPressed(AKEYCODE_SPACE).
// Missing features:
//  [ ] Platform: Clipboard support.
//  [ ] Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [ ] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'. FIXME: Check if this is even possible with Android.
// Important:
//  - FIXME: On-screen keyboard currently needs to be enabled by the application (see examples/ and issue #3446)
//  - FIXME: Unicode character inputs needs to be passed by Dear ImGui by the application (see examples/ and issue #3446)

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#pragma once

struct ANativeWindow;
struct AInputEvent;

IMGUI_IMPL_API bool     ImGui_ImplAndroid_Init(ANativeWindow* window , ImVec2 display_size);
IMGUI_IMPL_API int32_t  ImGui_ImplAndroid_HandleInputEvent(AInputEvent* input_event);
IMGUI_IMPL_API void     ImGui_ImplAndroid_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplAndroid_NewFrame(int screen_width = 0, int screen_height = 0);

#pragma once

#include <android/input.h>

struct ANativeWindow;
typedef struct{
    int Action;
    float x;
    float y;
    int VSCOLL;
    int HSCROLL;
    int ToolType;
    int ButtonState;
    float AXIS_VSCROLL;
    float AXIS_HSCROLL;
}MyInputEvent;

IMGUI_IMPL_API bool     ImGui_ImplAndroid_Init(ANativeWindow* window);
IMGUI_IMPL_API int32_t  ImGui_ImplAndroid_HandleInputEvent(MyInputEvent input_event);
IMGUI_IMPL_API int32_t  ImGui_ImplAndroid_HandleInputEvent(AInputEvent* input_event);

IMGUI_IMPL_API void     ImGui_ImplAndroid_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplAndroid_NewFrame();



#ifndef IMGUI_IMPL_ANDROID_GL2
#define IMGUI_IMPL_ANDROID_GL2

#include <imgui.h>

IMGUI_API bool        ImGui_ImplAndroidGLES2_Init();
IMGUI_API void        ImGui_ImplAndroidGLES2_Shutdown();
IMGUI_API void        ImGui_ImplAndroidGLES2_NewFrame();
//IMGUI_API bool        ImGui_ImplAndroidGLES2_ProcessEvent(SDL_Event* event);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void        ImGui_ImplAndroidGLES2_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_ImplAndroidGLES2_CreateDeviceObjects();
IMGUI_API void ImGui_ImplAndroidGLES2_RenderDrawLists(ImDrawData* draw_data);
#endif // IMGUI_IMPL_ANDROID_GL2
