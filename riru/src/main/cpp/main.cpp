#include <xhook/xhook.h>
#include <sys/system_properties.h>
#include "misc.h"
#include "jni_native_method.h"
#include "logging.h"
#include "wrap.h"
#include "module.h"
#include "api.h"
#include "native_method.h"
#include "hide_utils.h"
#include "status.h"
#include "config.h"

static int sdkLevel;
static int previewSdkLevel;
static char androidVersionName[PROP_VALUE_MAX + 1];

static JNINativeMethod *onRegisterZygote(
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {

    auto *newMethods = new JNINativeMethod[numMethods];
    memcpy(newMethods, methods, sizeof(JNINativeMethod) * numMethods);

    JNINativeMethod method;
    for (int i = 0; i < numMethods; ++i) {
        method = methods[i];

        if (strcmp(method.name, "nativeForkAndSpecialize") == 0) {
            JNI::Zygote::nativeForkAndSpecialize = new JNINativeMethod{method.name, method.signature, method.fnPtr};
            // 不同安卓版本之间的兼容
            if (strcmp(nativeForkAndSpecialize_r_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_r;
            else if (strcmp(nativeForkAndSpecialize_p_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_p;
            else if (strcmp(nativeForkAndSpecialize_oreo_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_oreo;
            else if (strcmp(nativeForkAndSpecialize_marshmallow_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_marshmallow;

            else if (strcmp(nativeForkAndSpecialize_r_dp3_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_r_dp3;
            else if (strcmp(nativeForkAndSpecialize_r_dp2_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_r_dp2;

            else if (strcmp(nativeForkAndSpecialize_q_alternative_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_q_alternative;

            else if (strcmp(nativeForkAndSpecialize_samsung_p_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_samsung_p;
            else if (strcmp(nativeForkAndSpecialize_samsung_o_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_samsung_o;
            else if (strcmp(nativeForkAndSpecialize_samsung_n_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_samsung_n;
            else if (strcmp(nativeForkAndSpecialize_samsung_m_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkAndSpecialize_samsung_m;

            else
                LOGW("found nativeForkAndSpecialize but signature %s mismatch", method.signature);

            auto replaced = newMethods[i].fnPtr != methods[i].fnPtr;
            if (replaced) {
                LOGI("replaced com.android.internal.os.Zygote#nativeForkAndSpecialize");
                api::setNativeMethodFunc(
                        get_modules()->at(0)->token, className, newMethods[i].name, newMethods[i].signature, newMethods[i].fnPtr);
            }
            Status::WriteMethod(Status::Method::forkAndSpecialize, replaced, method.signature);
        } else if (strcmp(method.name, "nativeSpecializeAppProcess") == 0) {
            JNI::Zygote::nativeSpecializeAppProcess = new JNINativeMethod{method.name, method.signature, method.fnPtr};

            if (strcmp(nativeSpecializeAppProcess_r_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeSpecializeAppProcess_r;
            else if (strcmp(nativeSpecializeAppProcess_q_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeSpecializeAppProcess_q;
            else if (strcmp(nativeSpecializeAppProcess_q_alternative_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeSpecializeAppProcess_q_alternative;
            else if (strcmp(nativeSpecializeAppProcess_sig_samsung_q, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeSpecializeAppProcess_samsung_q;

            else if (strcmp(nativeSpecializeAppProcess_r_dp3_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeSpecializeAppProcess_r_dp3;
            else if (strcmp(nativeSpecializeAppProcess_r_dp2_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeSpecializeAppProcess_r_dp2;

            else
                LOGW("found nativeSpecializeAppProcess but signature %s mismatch",
                     method.signature);

            auto replaced = newMethods[i].fnPtr != methods[i].fnPtr;
            if (replaced) {
                LOGI("replaced com.android.internal.os.Zygote#nativeSpecializeAppProcess");
                api::setNativeMethodFunc(
                        get_modules()->at(0)->token, className, newMethods[i].name, newMethods[i].signature, newMethods[i].fnPtr);
            }
            Status::WriteMethod(Status::Method::specializeAppProcess, replaced, method.signature);
        } else if (strcmp(method.name, "nativeForkSystemServer") == 0) {
            JNI::Zygote::nativeForkSystemServer = new JNINativeMethod{method.name, method.signature, method.fnPtr};

            if (strcmp(nativeForkSystemServer_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkSystemServer;
            else if (strcmp(nativeForkSystemServer_samsung_q_sig, method.signature) == 0)
                newMethods[i].fnPtr = (void *) nativeForkSystemServer_samsung_q;
            else
                LOGW("found nativeForkSystemServer but signature %s mismatch", method.signature);

            auto replaced = newMethods[i].fnPtr != methods[i].fnPtr;
            if (replaced) {
                LOGI("replaced com.android.internal.os.Zygote#nativeForkSystemServer");
                api::setNativeMethodFunc(
                        get_modules()->at(0)->token, className, newMethods[i].name, newMethods[i].signature, newMethods[i].fnPtr);
            }
            Status::WriteMethod(Status::Method::forkSystemServer, replaced, method.signature);
        }
    }

    return newMethods;
}

static JNINativeMethod *onRegisterSystemProperties(
        JNIEnv *env, const char *className, const JNINativeMethod *methods, int numMethods) {

    auto *newMethods = new JNINativeMethod[numMethods];
    memcpy(newMethods, methods, sizeof(JNINativeMethod) * numMethods);

    JNINativeMethod method;
    for (int i = 0; i < numMethods; ++i) {
        method = methods[i];

        if (strcmp(method.name, "native_set") == 0) {
            JNI::SystemProperties::set = new JNINativeMethod{method.name, method.signature, method.fnPtr};

            if (strcmp("(Ljava/lang/String;Ljava/lang/String;)V", method.signature) == 0)
                newMethods[i].fnPtr = (void *) SystemProperties_set;
            else
                LOGW("found native_set but signature %s mismatch", method.signature);

            if (newMethods[i].fnPtr != methods[i].fnPtr) {
                LOGI("replaced android.os.SystemProperties#native_set");

                api::setNativeMethodFunc(
                        get_modules()->at(0)->token, className, newMethods[i].name, newMethods[i].signature, newMethods[i].fnPtr);
            }
        }
    }
    return newMethods;
}

#define XHOOK_REGISTER(PATH_REGEX, NAME) \
    if (xhook_register(PATH_REGEX, #NAME, (void*) new_##NAME, (void **) &old_##NAME) != 0) \
        LOGE("failed to register hook " #NAME "."); \

#define NEW_FUNC_DEF(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

NEW_FUNC_DEF(int, jniRegisterNativeMethods, JNIEnv *env, const char *className,
             const JNINativeMethod *methods, int numMethods) {
    api::putNativeMethod(className, methods, numMethods);

    LOGD("jniRegisterNativeMethods %s", className);

    JNINativeMethod *newMethods = nullptr;
    if (strcmp("com/android/internal/os/Zygote", className) == 0) {
        // com/android/internal/os/Zygote注册时回调onRegisterZygote方法获取新的jniMethods列表进行替换
        newMethods = onRegisterZygote(env, className, methods, numMethods);
    } else if (strcmp("android/os/SystemProperties", className) == 0) {
        // hook android.os.SystemProperties#native_set to prevent a critical problem on Android 9
        // see comment of SystemProperties_set in jni_native_method.cpp for detail
        // 作者提到：安卓9之后，SystemProperties.set("sys.user." + userId + ".ce_available", "true")可能会小概率出现异常，虽然不确定是否riru导致
        // 回调onRegisterSystemProperties方法
        newMethods = onRegisterSystemProperties(env, className, methods, numMethods);
    }

    int res = old_jniRegisterNativeMethods(env, className, newMethods ? newMethods : methods,
                                           numMethods);
    /*if (!newMethods) {
        NativeMethod::jniRegisterNativeMethodsPost(env, className, methods, numMethods);
    }*/
    delete newMethods;
    return res;
}

void restore_replaced_func(JNIEnv *env) {
    xhook_register(".*\\libandroid_runtime.so$", "jniRegisterNativeMethods",
                   (void *) old_jniRegisterNativeMethods,
                   nullptr);
    if (xhook_refresh(0) == 0) {
        xhook_clear();
        LOGD("hook removed");
    }

#define restoreMethod(cls, method) \
    if (JNI::cls::method != nullptr) { \
        old_jniRegisterNativeMethods(env, JNI::cls::classname, JNI::cls::method, 1); \
        delete JNI::cls::method; \
    }

    restoreMethod(Zygote, nativeForkAndSpecialize)
    restoreMethod(Zygote, nativeSpecializeAppProcess)
    restoreMethod(Zygote, nativeForkSystemServer)
    restoreMethod(SystemProperties, set)
}

static void read_prop() {
    char sdk[PROP_VALUE_MAX + 1];
    if (__system_property_get("ro.build.version.sdk", sdk) > 0)
        sdkLevel = atoi(sdk);

    if (__system_property_get("ro.build.version.preview_sdk", sdk) > 0)
        previewSdkLevel = atoi(sdk);

    __system_property_get("ro.build.version.release", androidVersionName);

    LOGI("system version %s (api %d, preview_sdk %d)", androidVersionName, sdkLevel, previewSdkLevel);
}

extern "C" void constructor() __attribute__((constructor));

// _init_array libriru.so被dlopen后最先执行的函数
void constructor() {
#ifdef DEBUG_APP
    hide::hide_modules(nullptr, 0);
#endif

    if (getuid() != 0)
        return;

    char cmdline[ARG_MAX + 1];
    get_self_cmdline(cmdline, 0);

    if (strcmp(cmdline, "zygote") != 0
        && strcmp(cmdline, "zygote32") != 0
        && strcmp(cmdline, "zygote64") != 0
        && strcmp(cmdline, "usap32") != 0
        && strcmp(cmdline, "usap64") != 0) {
        LOGW("not zygote (cmdline=%s)", cmdline);
        return;
    }

    LOGI("Riru %s (%d) in %s", RIRU_VERSION_NAME, RIRU_VERSION_CODE, cmdline);

    LOGI("config dir is %s", CONFIG_DIR);

    if (access(CONFIG_DIR "/disable", F_OK) == 0) {
        LOGI("%s exists, do nothing", CONFIG_DIR "/disable");
        return;
    }

    read_prop();

    // hook libandroid_runtime.so中jniRegisterNativeMethods方法，因为libandroid_runtime.so中所有JNI方法都是通过该方法进行注册
    // 因此通过hook该方法可以在com.android.internal.os.Zygote#nativeForkAndSpecialize和com.android.internal.os.Zygote#nativeForkSystemServer注册时进行替换
    XHOOK_REGISTER(".*\\libandroid_runtime.so$", jniRegisterNativeMethods);

    if (xhook_refresh(0) == 0) {
        xhook_clear();
        LOGI("hook installed");
    } else {
        LOGE("failed to refresh hook");
    }

    // 加载插件
    load_modules();

    Status::WriteSelfAndModules();
}