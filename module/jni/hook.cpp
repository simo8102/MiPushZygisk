#include "hook.h"
#include "logging.h"
#include <string>

using namespace std;

jstring (*orig_native_get)(JNIEnv *env, jclass clazz, jstring keyJ, jstring defJ);

jstring my_native_get(JNIEnv *env, jclass clazz, jstring keyJ, jstring defJ) {
    const char *key = env->GetStringUTFChars(keyJ, nullptr);
    const char *def = env->GetStringUTFChars(defJ, nullptr);

    jstring hooked_result = nullptr;

    if (strcmp(key, "ro.miui.ui.version.name") == 0) {
        hooked_result = env->NewStringUTF("V130");
    } else if (strcmp(key, "ro.miui.ui.version.code") == 0) {
        hooked_result = env->NewStringUTF("13");
    }

    env->ReleaseStringUTFChars(keyJ, key);
    env->ReleaseStringUTFChars(defJ, def);

    if (hooked_result != nullptr) {
        return hooked_result;
    } else {
        return orig_native_get(env, clazz, keyJ, defJ);
    }
}

void hookBuild(JNIEnv *env) {
    LOGD("hook Build\n");
    jclass build_class = env->FindClass("android/os/Build");
    jstring new_brand = env->NewStringUTF("Xiaomi");
    jstring new_manufacturer = env->NewStringUTF("Xiaomi");

    jfieldID brand_id = env->GetStaticFieldID(build_class, "BRAND", "Ljava/lang/String;");
    if (brand_id != nullptr) {
        env->SetStaticObjectField(build_class, brand_id, new_brand);
    }

    jfieldID manufacturer_id = env->GetStaticFieldID(build_class, "MANUFACTURER", "Ljava/lang/String;");
    if (manufacturer_id != nullptr) {
        env->SetStaticObjectField(build_class, manufacturer_id, new_manufacturer);
    }

    env->DeleteLocalRef(new_brand);
    env->DeleteLocalRef(new_manufacturer);

    LOGD("hook Build done");
}

void hookSystemProperties(JNIEnv *env, zygisk::Api *api) {
    LOGD("hook SystemProperties\n");

    JNINativeMethod targetHookMethods[] = {
            {"native_get", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
             (void *) my_native_get},
    };

    api->hookJniNativeMethods(env, "android/os/SystemProperties", targetHookMethods, 1);

    *(void **) &orig_native_get = targetHookMethods[0].fnPtr;

    LOGD("hook SystemProperties done: %p\n", orig_native_get);
}

void Hook::hook() {
    hookBuild(env);
    hookSystemProperties(env, api);
}
