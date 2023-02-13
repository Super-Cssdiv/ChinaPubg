#声明要求的cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

project(ProjectName)

if(${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(ARCH "arm64")
elseif(${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(ARCH "arm")
endif()

#设置编译版本
set(CMAKE_BUILD_TYPE "Release")
#去除所有符号
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
#导出指定函数
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${CMAKE_SOURCE_DIR}/export.map")

add_definitions(-DMODULE_VERSION_NAME="${MODULE_VERSION_NAME}")
add_definitions(-DMODULE_NAME="lib${MODULE_NAME}.so")

add_definitions(-DMODULE_URL="${MODULE_URL}")
add_definitions(-DMODULE_URL_BAK="${MODULE_URL_BAK}")
add_definitions(-DMODULE_KEY="${MODULE_KEY}")
add_definitions(-DMODULE_URL_BASE="${MODULE_URL_BASE}")
add_definitions(-DGAME_VERSION_CODE="${GAME_VERSION_CODE}")

enable_language(C ASM)

set(asm_syscall lib/${ANDROID_ABI}/syscall.s)

include_directories(
        src
        include
        include/ConfigManger
        include/filedata
        include/netlink
        include/imgui
        include/cJson
        include/strnormalize
        include/MapTools
        include/MemoryTools
        include/Logger
        include/RecorderTools
        include/curl
        include/HookFramework
        include/PlatformUtil
        include/TencentSafe
        include/searchCall
        include/PtraceInject
        include/HideLibrary
        include/MainLooper)

include_directories(include/shadowhook
        include/shadowhook/common
        include/shadowhook/arch/${ARCH})

include_directories(include/third_party/bsd
        include/third_party/lss
        include/third_party/xdl)

file(GLOB xdl include/third_party/xdl/*.c)
file(GLOB shadowhook include/shadowhook/*.c include/shadowhook/arch/${ARCH}/*.c include/shadowhook/common/*.c)
file(GLOB bhook include/bhook/*.c)
file(GLOB libseccomp include/SeccompTools/*.c)

add_library(local_curl STATIC IMPORTED)
set_target_properties(local_curl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ANDROID_ABI}/libcurl.a)

add_library(
        ${MODULE_NAME}

        SHARED

        MainEntry.cpp

        src/Draw.cpp
        src/Menu.cpp
        src/GameUtils.cpp

        include/cJson/cJSON.c
        include/strnormalize/strnormalize.cpp
        include/MapTools/MapTools.cpp
        include/MemoryTools/MemoryTools.cpp

        include/imgui/imgui.cpp
        include/imgui/imgui_expand.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_tables.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/stb_image.cpp
        include/imgui/backends/imgui_impl_android.cpp
        include/imgui/backends/imgui_impl_opengl3.cpp

        include/netlink/bionic_netlink.cpp
        include/netlink/ifaddrs.cpp

        include/RecorderTools/RecorderTools.cpp
        include/MainLooper/MainLooper.cpp

        include/ConfigManger/ConfigManger.cpp

        include/filedata/drawicon.cpp
        include/filedata/font.cpp

        include/base64/base64.cpp

        include/HookFramework/hde64.c
        include/HookFramework/SubstrateDebug.cpp
        include/HookFramework/SubstrateHook.cpp
        include/HookFramework/SubstratePosixMemory.cpp
        include/HookFramework/And64InlineHook.cpp
        include/HookFramework/elf_util.cpp

        #include/TencentSafe/TencentSafe.cpp

        include/searchCall/searchCall.cpp
        include/HideLibrary/hide_utils.cpp
        include/HideLibrary/byopen_android.c

        ${xdl}

        ${bhook}

        ${asm_syscall}
)

#[[add_library(
        HideLibrary

        SHARED

        include/HideLibrary/HideLibrary.cpp

        include/MapTools/MapTools.cpp

        include/HookFramework/elf_util.cpp

        include/HideLibrary/hide_utils.cpp
)]]

add_executable(
        PeaceTools

        include/MapTools/MapTools.cpp

        include/PtraceInject/ptrace_utils.cpp
        include/PtraceInject/inject.cpp
        include/PtraceInject/PtraceInject.cpp
)

target_link_libraries(${MODULE_NAME}
        GLESv3
        android
        local_curl
        EGL
        log
        )

#target_link_libraries(HideLibrary log)

set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)