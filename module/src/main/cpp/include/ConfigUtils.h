#pragma once

#include "Draw.h"
#include <fstream>
#include <iostream>

string InputPwd;
string loginInfo;
bool isAutoLogin = false;
ImVec2 SaveWindowPos = {-520,-520};//NOLINT
int viewType = 1;
string host = MODULE_URL;

bool parseConfig(const char* jsonBuf) {
    return false;
}

bool updateConfig() {
    return false;
}

bool getConfig() {
    return false;
}