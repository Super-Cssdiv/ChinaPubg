#include <RecorderTools.h>
#include <jni.h>
#include <string>
#include <sys/system_properties.h>

using namespace std;

static string sName;

string getProp(const char *strCmd) {
    char prop_value[PROP_VALUE_MAX] = {0};
    __system_property_get(strCmd, prop_value);
    string strResult(prop_value);
    return strResult;
}

// 不区分大小写
static bool equals(const char *str1, const char *str2) {
    if (str1 == nullptr && str2 == nullptr) {
        return true;
    } else {
        if (str1 != nullptr && str2 != nullptr) {
            return strcasecmp(str1, str2) == 0;
        } else {
            return false;
        }
    }
}

size_t find(string strSource, const char* szTarget){
    if(strSource.empty()){
        return string::npos;
    }
    string strSub = szTarget;
    if (strSub.empty()){
        return string::npos;
    }

    for (char & it : strSource){
        it = tolower(it);
    }

    for (char & ite : strSub){
        ite=tolower(ite);//do not change szTarget context.
    }

    return strSource.find(strSub);
}


bool check(const char * rom) {
    return false;
}

const char* getFakeRecordWindowTitle() {
    return "";
}

void setFakeRecorderWindowLayoutParams(JNIEnv *env, jobject layoutParams) {
    
}