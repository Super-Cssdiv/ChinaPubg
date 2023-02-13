#pragma once

#include <string>
#include <Structs.h>
#include <cJSON.h>


string getName(uintptr_t object);
void *updateDataList(void *);
void createDataList() ;
void setGameOffset(cJSON* cjson);
android_app *getAndroidApp();
DisplayMetrics getGameScreenInfo();
int getMatrixValue();
int getActorCount();