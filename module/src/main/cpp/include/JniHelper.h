#pragma once

android_app *g_App = nullptr;

// JNI配置
JavaVM* CurrentJavaVM = nullptr;
bool isFakeRecorder = false;

void JavaEnvDestructor(void*) {
    CurrentJavaVM->DetachCurrentThread();
}

JNIEnv* GetJavaEnv() {
    static uintptr_t TlsSlot = 0;
    if (TlsSlot == 0) {
        pthread_key_create((pthread_key_t*)&TlsSlot, &JavaEnvDestructor);
    }
    auto* Env = (JNIEnv*)pthread_getspecific(TlsSlot);
    if (Env == nullptr) {
        CurrentJavaVM->GetEnv((void **)&Env, JNI_VERSION_1_6);
        jint AttachResult = CurrentJavaVM->AttachCurrentThread(&Env, nullptr);
        if (AttachResult == JNI_ERR) {
            return nullptr;
        }
        pthread_setspecific(TlsSlot, (void*)Env);
    }
    return Env;
}

/* 弹出输入法 */
void ShowSoftKeyboardInput(int InputType, const char* Label, const char* Contents) {

    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    auto LabelJava = java_env->NewStringUTF(Label);

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "AndroidThunkJava_ShowVirtualKeyboardInput", "(ILjava/lang/String;Ljava/lang/String;)V");
    auto ContentsJava = java_env->NewStringUTF(Contents);
    java_env->CallVoidMethod(g_App->activity->clazz, method_id, InputType, LabelJava, ContentsJava);

    java_env->DeleteLocalRef(LabelJava);
    java_env->DeleteLocalRef(ContentsJava);
    java_env->DeleteLocalRef(native_activity_clazz);
}

void passLoginFase() {
    JNIEnv *java_env = GetJavaEnv();
    jclass WebViewXClass = java_env->FindClass("com/tencent/msdk/webviewx/core/WebViewX");
    jmethodID getInstanceMtd = java_env->GetStaticMethodID(WebViewXClass, "getInstance","()Lcom/tencent/msdk/webviewx/core/WebViewX;");
    jobject getInstance = java_env->CallStaticObjectMethod(WebViewXClass, getInstanceMtd);
    jmethodID setCloseMsgMtd = java_env->GetMethodID(WebViewXClass, "setCloseMsg","(Ljava/lang/String;)V");
    jstring str = java_env->NewStringUTF(R"({'flag':0,' desc':'h5 real name auth succeed'})");
    java_env->CallVoidMethod(getInstance, setCloseMsgMtd, str);
    java_env->DeleteLocalRef(str);
    java_env->DeleteLocalRef(WebViewXClass);
}



/* 获取输入法内容 */
string GetSoftKeyboardInput() {
    string result;
    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    jfieldID jfield_VirtualKeyboardInput = java_env->GetFieldID(native_activity_clazz, "newVirtualKeyboardInput", "Lcom/epicgames/ue4/GameActivity$VirtualKeyboardInput;");
    jobject newVirtualKeyboardInput = java_env->GetObjectField(g_App->activity->clazz, jfield_VirtualKeyboardInput);
    jclass native_edittext_clazz = java_env->GetObjectClass(newVirtualKeyboardInput);
    jmethodID jmethod_getText = java_env->GetMethodID(native_edittext_clazz, "getText","()Landroid/text/Editable;");
    jobject text = java_env->CallObjectMethod(newVirtualKeyboardInput, jmethod_getText);
    if (text) {
        jclass native_editable_clazz = java_env->FindClass("android/text/Editable");
        jmethodID jmethod_toString = java_env->GetMethodID(native_editable_clazz, "toString","()Ljava/lang/String;");
        auto str = (jstring) java_env->CallObjectMethod(text, jmethod_toString);
        result = java_env->GetStringUTFChars(str, nullptr);
        java_env->DeleteLocalRef(str);
        java_env->DeleteLocalRef(text);
        java_env->DeleteLocalRef(native_editable_clazz);
    }
    java_env->DeleteLocalRef(newVirtualKeyboardInput);
    java_env->DeleteLocalRef(native_edittext_clazz);
    java_env->DeleteLocalRef(native_activity_clazz);
    return result;
}

/* 输入法是否弹出 */
bool bKeyboardShowing() {
    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    jfieldID jfield_bKeyboardShowing = java_env->GetFieldID(native_activity_clazz, "bKeyboardShowing", "Z");
    jboolean ret = java_env->GetBooleanField(g_App->activity->clazz, jfield_bKeyboardShowing);
    java_env->DeleteLocalRef(native_activity_clazz);
    return ret;
}

/* 弹出Toast */
void showMessage(const char* Contents) {
    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    jfieldID jfield_bleAdapterWrapper = java_env->GetFieldID(native_activity_clazz, "bleAdapterWrapper", "Lcom/tencent/tmgp/pubgmhd/JNIBLEAdapterWrapper;");
    jobject bleAdapterWrapper = java_env->GetObjectField(g_App->activity->clazz, jfield_bleAdapterWrapper);

    jclass JNIBLEAdapterWrapper_clazz = java_env->GetObjectClass(bleAdapterWrapper);
    jfieldID jfield_blePeripheral = java_env->GetFieldID(JNIBLEAdapterWrapper_clazz, "blePeripheral", "Lcom/tencent/tmgp/pubgmhd/BLEPeripheral;");
    jobject blePeripheral = java_env->GetObjectField(bleAdapterWrapper, jfield_blePeripheral);

    jclass BLEPeripheral_clazz = java_env->GetObjectClass(blePeripheral);
    jmethodID method_id = java_env->GetMethodID(BLEPeripheral_clazz, "showMessage", "(Ljava/lang/String;)V");
    jstring str = java_env->NewStringUTF(Contents);
    java_env->CallVoidMethod(blePeripheral, method_id, str);

    java_env->DeleteLocalRef(str);
    java_env->DeleteLocalRef(native_activity_clazz);
    java_env->DeleteLocalRef(bleAdapterWrapper);
    java_env->DeleteLocalRef(JNIBLEAdapterWrapper_clazz);
    java_env->DeleteLocalRef(blePeripheral);
    java_env->DeleteLocalRef(BLEPeripheral_clazz);
}

/* 检查应用权限 */
bool checkPermission(const char* Contents) {
    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "AndroidThunkJava_CheckPermission", "(Ljava/lang/String;)Z");
    jstring str = java_env->NewStringUTF(Contents);
    jboolean ret = java_env->CallBooleanMethod(g_App->activity->clazz, method_id, str);

    java_env->DeleteLocalRef(str);
    java_env->DeleteLocalRef(native_activity_clazz);
    return ret;
}

/* 获取屏幕分辨率 */
DisplayMetrics getScreenInfo() {
    //获取window对象

    DisplayMetrics result;
    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    jmethodID getSystemServiceMethod = java_env->GetMethodID(native_activity_clazz, "getSystemService","(Ljava/lang/String;)Ljava/lang/Object;");

    jstring str = java_env->NewStringUTF("window");
    jobject windowManager = java_env->CallObjectMethod(g_App->activity->clazz, getSystemServiceMethod, str);
    java_env->DeleteLocalRef(str);

    jclass PointClass = java_env->FindClass("android/graphics/Point");
    jmethodID pointInit = java_env->GetMethodID(PointClass, "<init>", "()V");
    jobject point = java_env->NewObject(PointClass, pointInit);

    jclass windowManagerClass = java_env->FindClass("android/view/WindowManager");
    jmethodID getDefaultDisplay = java_env->GetMethodID(windowManagerClass, "getDefaultDisplay","()Landroid/view/Display;");
    jobject display = java_env->CallObjectMethod(windowManager, getDefaultDisplay);

    jclass displayClass = java_env->GetObjectClass(display);
    jmethodID getRealSize = java_env->GetMethodID(displayClass, "getRealSize","(Landroid/graphics/Point;)V");
    java_env->CallVoidMethod(display, getRealSize, point);

    jfieldID xField = java_env->GetFieldID(PointClass, "x", "I");
    jint x = java_env->GetIntField(point, xField);

    jfieldID yField = java_env->GetFieldID(PointClass, "y", "I");
    jint y = java_env->GetIntField(point, yField);

    if (x > y) {
        result.widthPixels = x;
        result.heightPixels = y;
    } else {
        result.widthPixels = y;
        result.heightPixels = x;
    }

    java_env->DeleteLocalRef(native_activity_clazz);
    java_env->DeleteLocalRef(native_activity_clazz);
    java_env->DeleteLocalRef(PointClass);
    java_env->DeleteLocalRef(point);
    java_env->DeleteLocalRef(windowManagerClass);
    java_env->DeleteLocalRef(display);
    java_env->DeleteLocalRef(displayClass);
    return result;
}

int dip2px(float dpValue) {
    int result;
    JNIEnv* java_env = GetJavaEnv();
    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    jmethodID getResourcesMethod = java_env->GetMethodID(native_activity_clazz, "getResources","()Landroid/content/res/Resources;");
    jobject Resources = java_env->CallObjectMethod(g_App->activity->clazz, getResourcesMethod);
    jclass ResourcesClass = java_env->FindClass("android/content/res/Resources");
    jmethodID getDisplayMetricsMethod = java_env->GetMethodID(ResourcesClass, "getDisplayMetrics","()Landroid/util/DisplayMetrics;");
    jobject DisplayMetrics = java_env->CallObjectMethod(Resources, getDisplayMetricsMethod);
    jclass displayClass = java_env->GetObjectClass(DisplayMetrics);
    jfieldID densityField = java_env->GetFieldID(displayClass, "density", "F");
    float scale  = java_env->GetFloatField(DisplayMetrics, densityField);
    result = (int) (dpValue * scale + 0.5f);
    java_env->DeleteLocalRef(native_activity_clazz);
    java_env->DeleteLocalRef(ResourcesClass);
    java_env->DeleteLocalRef(displayClass);
    java_env->DeleteLocalRef(displayClass);
    return result;
}

//public LayoutParams(int w, int h, int xpos, int ypos, int _type,
//                int _flags, int _format)
jobject createLayoutParams(int width, int height) {
    JNIEnv *env = GetJavaEnv();
    jclass lpClass = env->FindClass("android/view/WindowManager$LayoutParams");
    jmethodID lpInit = env->GetMethodID(lpClass, "<init>", "()V");
    jobject lp = env->NewObject(lpClass, lpInit);

    jfieldID typeField = env->GetFieldID(lpClass, "type", "I");
    env->SetIntField(lp, typeField, 2038);
//
   jfieldID flagsField = env->GetFieldID(lpClass, "flags", "I");
    env->SetIntField(lp, flagsField, 512 | 8 | 16 | 32 | 1024 | 256 | 0x01000000 | 1073741824); //824 隐藏窗口前flags:16778040 隐藏窗口后flags:16782136 8192

    jfieldID gravityField = env->GetFieldID(lpClass, "gravity", "I");
    env->SetIntField(lp, gravityField, 3 | 48);//Gravity.LEFT | Gravity.TOP
   // env->SetIntField(lp, gravityField, 8388611);

    jfieldID formatField = env->GetFieldID(lpClass, "format", "I");
    env->SetIntField(lp, formatField, 1);//PixelFormat.RGBA_8888

    jfieldID widthField = env->GetFieldID(lpClass, "width", "I");
    env->SetIntField(lp, widthField, width);

    jfieldID heightField = env->GetFieldID(lpClass, "height", "I");
    env->SetIntField(lp, heightField, height);


    /*if (checkPermission("android.permission.INTERNAL_SYSTEM_WINDOW") || checkPermission("flyme.permission.SKIP_CAPTURE")) {
        setFakeRecorderWindowLayoutParams(env, lp);
        isFakeRecorder = true;
    }*/
    env-> DeleteLocalRef(lpClass);
    return lp;
}

jobject createDrawView(DisplayMetrics size) {
    JNIEnv *env = GetJavaEnv();
    //获取window对象
    jclass mainClass = env->GetObjectClass(g_App->activity->clazz);
    //android.app.Activity
    jmethodID getApplicationMethod = env->GetMethodID(mainClass, "getApplication", "()Landroid/app/Application;");
    jobject application = env->CallObjectMethod(g_App->activity->clazz, getApplicationMethod);
    jclass applicationClass = env->GetObjectClass(application);
    jmethodID getSystemService = env->GetMethodID(applicationClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring windowStr = env->NewStringUTF("window");
    jobject windowManager = env->CallObjectMethod(application, getSystemService, windowStr);
    env->DeleteLocalRef(windowStr);
    jclass windowManagerClass = env->GetObjectClass(windowManager);

    //创建布局参数
    jobject lp = createLayoutParams(size.widthPixels, size.heightPixels);

    if (!isFakeRecorder) {
        //创建surfaceView
        jclass surfaceViewClass = env->FindClass("android/view/SurfaceView");
        jmethodID surfaceViewInit = env->GetMethodID(surfaceViewClass, "<init>", "(Landroid/content/Context;)V");
        jobject surfaceView = env->NewObject(surfaceViewClass, surfaceViewInit, g_App->activity->clazz);
        jmethodID addView = env->GetMethodID(windowManagerClass, "addView", "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");

        jmethodID getHolderMethod = env->GetMethodID(surfaceViewClass, "getHolder", "()Landroid/view/SurfaceHolder;");
        jobject surfaceHolder = env->CallObjectMethod(surfaceView, getHolderMethod);
        jclass surfaceHolderClass = env->GetObjectClass(surfaceHolder);
        jmethodID setZOrderOnTopMethod = env->GetMethodID(surfaceViewClass, "setZOrderOnTop", "(Z)V");
        env->CallVoidMethod(surfaceView, setZOrderOnTopMethod, true);
        jmethodID setFormatMethod = env->GetMethodID(surfaceHolderClass, "setFormat", "(I)V");
        env->CallVoidMethod(surfaceHolder, setFormatMethod, -2);
        env->CallVoidMethod(windowManager, addView, surfaceView, lp);

        env-> DeleteLocalRef(mainClass);
        //env-> DeleteLocalRef(application);
        env-> DeleteLocalRef(applicationClass);
        env-> DeleteLocalRef(windowManager);
        env-> DeleteLocalRef(surfaceViewClass);
        env-> DeleteLocalRef(surfaceHolder);
        env-> DeleteLocalRef(surfaceHolderClass);
        env-> DeleteLocalRef(surfaceHolderClass);
        return env->NewGlobalRef(surfaceView);
    }

    //创建TextrueView
    jclass textureViewClass = env->FindClass("android/view/TextureView");
    jmethodID textureViewInit = env->GetMethodID(textureViewClass, "<init>", "(Landroid/content/Context;)V");
    jobject textureView = env->NewObject(textureViewClass, textureViewInit, g_App->activity->clazz);
    jmethodID addView = env->GetMethodID(windowManagerClass, "addView", "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
    env->CallVoidMethod(windowManager, addView, textureView, lp);
    env-> DeleteLocalRef(mainClass);
    env-> DeleteLocalRef(application);
    env-> DeleteLocalRef(applicationClass);
    env-> DeleteLocalRef(windowManager);
    env-> DeleteLocalRef(textureViewClass);
    return env->NewGlobalRef(textureView);
}

bool canDrawOverlays() {
    JNIEnv* java_env = GetJavaEnv();
    jclass ContextClass = java_env->FindClass("android/provider/Settings");
    jmethodID Method_canDrawOverlays = java_env->GetStaticMethodID(ContextClass, "canDrawOverlays", "(Landroid/content/Context;)Z");
    bool result = java_env->CallStaticBooleanMethod(ContextClass, Method_canDrawOverlays, g_App->activity->clazz);
    java_env-> DeleteLocalRef(ContextClass);
    return result;
}


