#pragma once
#include <jni.h>
#include <string>
#include <sys/system_properties.h>
#include <Logger.h>

using namespace std;

static const char * ROM_MIUI = "MIUI";
static const char * ROM_EMUI = "EMUI";
static const char * ROM_FLYME = "FLYME";
static const char * ROM_OPPO = "OPPO";
static const char * ROM_SMARTISAN = "SMARTISAN";
static const char * ROM_VIVO = "VIVO";
static const char * ROM_QIKU = "QIKU";
static const char * ROM_NUBIAUI = "NUBIAUI";
static const char * ROM_ONEPLUS = "HYDROGEN";
static const char * ROM_SAMSUNG = "ONEUI";
static const char * ROM_BLACKSHARK = "JOYUI";
static const char * ROM_ROG = "ASUS";
static const char * ROM_LENOVO = "ZUI";
static const char * ROM_REALME = "realmeui";
static const char * ROM_COLOR = "COLOR";

static const char * KEY_VERSION_MIUI = "ro.miui.ui.version.name";
static const char * KEY_VERSION_EMUI = "ro.build.version.emui";
static const char * KEY_VERSION_OPPO = "ro.build.version.opporom";
static const char * KEY_VERSION_SMARTISAN = "ro.smartisan.version";
static const char * KEY_VERSION_VIVO = "ro.vivo.os.version";
static const char * KEY_VERSION_NUBIA = "ro.build.nubia.rom.name";
static const char * KEY_VERSION_ONEPLIS = "ro.build.ota.versionname";
static const char * KEY_VERSION_SAMSUNG = "ro.channel.officehubrow";
static const char * KEY_VERSION_BLACKSHARK = "ro.blackshark.rom";
static const char * KEY_VERSION_ROG = "ro.asus.rog";
static const char * KEY_VERSION_LENOVO = "ro.zuk.product.market";
static const char * KEY_VERSION_REALME = "ro.build.version.realmeui";
static const char * KEY_VERSION_COLOR = "ro.build.version.oplusrom";
static const char * KEY_VERSION_FLYME = "ro.flyme.published";


bool check(const char * rom);

const char* getFakeRecordWindowTitle();

void setFakeRecorderWindowLayoutParams(JNIEnv *env, jobject layoutParams);