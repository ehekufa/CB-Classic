// native_ui.cpp — мост к android/app/src/main/java/org/cbclassic/NativeUi.java.
//
// Из нативного потока нельзя просто вызвать FindClass: он не видит классы
// приложения, поэтому класс ищется через ClassLoader активности (тот же приём,
// что и в fbnet.cpp). Ответ диалога приходит с UI-потока, поэтому он кладётся
// в очередь под мьютексом, а движок забирает его в кадре.
#include "native_ui.hpp"

#include <deque>
#include <mutex>
#include <utility>

#if defined(__ANDROID__) && defined(CB_USE_SDL2)

#include <jni.h>

#include <SDL2/SDL_system.h>

namespace {

std::mutex g_mu;
std::deque<std::pair<std::string, bool>> g_results;   // (текст, «Принять»)
jclass g_class = nullptr;                             // org.cbclassic.NativeUi

// Класс NativeUi через ClassLoader активности (кэшируется глобальной ссылкой).
jclass ui_class(JNIEnv *env, jobject activity)
{
    if (g_class) {
        return g_class;
    }
    jclass cls = nullptr;
    jclass act_cls = env->GetObjectClass(activity);
    if (act_cls) {
        jmethodID get_loader =
            env->GetMethodID(act_cls, "getClassLoader", "()Ljava/lang/ClassLoader;");
        jobject loader = get_loader ? env->CallObjectMethod(activity, get_loader) : nullptr;
        if (loader) {
            jclass loader_cls = env->GetObjectClass(loader);
            jmethodID load = loader_cls
                ? env->GetMethodID(loader_cls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;")
                : nullptr;
            if (load) {
                jstring name = env->NewStringUTF("org.cbclassic.NativeUi");
                cls = static_cast<jclass>(env->CallObjectMethod(loader, load, name));
                env->DeleteLocalRef(name);
            }
        }
    }
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        cls = nullptr;
    }
    if (cls) {
        g_class = static_cast<jclass>(env->NewGlobalRef(cls));
    }
    return g_class;
}

bool ui_env(JNIEnv **out)
{
    JNIEnv *env = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
    jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !activity) {
        return false;
    }
    if (!ui_class(env, activity)) {
        return false;
    }
    *out = env;
    return true;
}

}  // namespace

// Ответ диалога из Java. Вызывается на UI-потоке, поэтому только очередь.
extern "C" JNIEXPORT void JNICALL
Java_org_cbclassic_NativeUi_nativeResult(JNIEnv *env, jclass, jstring text)
{
    std::string value;
    bool accepted = false;
    if (text) {
        const char *chars = env->GetStringUTFChars(text, nullptr);
        if (chars) {
            value = chars;
            accepted = true;
            env->ReleaseStringUTFChars(text, chars);
        }
    }
    std::lock_guard<std::mutex> lock(g_mu);
    g_results.emplace_back(value, accepted);
}

namespace cb {

bool native_ui_available()
{
    JNIEnv *env = nullptr;
    return ui_env(&env);
}

void native_toast(const std::string &text)
{
    JNIEnv *env = nullptr;
    if (!ui_env(&env)) {
        return;
    }
    jmethodID method = env->GetStaticMethodID(g_class, "toast", "(Ljava/lang/String;)V");
    if (!method || env->ExceptionCheck()) {
        env->ExceptionClear();
        return;
    }
    jstring message = env->NewStringUTF(text.c_str());
    env->CallStaticVoidMethod(g_class, method, message);
    env->DeleteLocalRef(message);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}

void native_ask(const std::string &title, const std::string &hint,
                const std::string &initial, const std::string &ok,
                const std::string &cancel)
{
    JNIEnv *env = nullptr;
    if (!ui_env(&env)) {
        return;
    }
    jmethodID method = env->GetStaticMethodID(
        g_class, "ask",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
        "Ljava/lang/String;)V");
    if (!method || env->ExceptionCheck()) {
        env->ExceptionClear();
        return;
    }
    jstring jtitle = env->NewStringUTF(title.c_str());
    jstring jhint = env->NewStringUTF(hint.c_str());
    jstring jinitial = env->NewStringUTF(initial.c_str());
    jstring jok = env->NewStringUTF(ok.c_str());
    jstring jcancel = env->NewStringUTF(cancel.c_str());
    env->CallStaticVoidMethod(g_class, method, jtitle, jhint, jinitial, jok, jcancel);
    env->DeleteLocalRef(jtitle);
    env->DeleteLocalRef(jhint);
    env->DeleteLocalRef(jinitial);
    env->DeleteLocalRef(jok);
    env->DeleteLocalRef(jcancel);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}

void native_ask_dismiss()
{
    JNIEnv *env = nullptr;
    if (!ui_env(&env)) {
        return;
    }
    jmethodID method = env->GetStaticMethodID(g_class, "dismiss", "()V");
    if (!method || env->ExceptionCheck()) {
        env->ExceptionClear();
        return;
    }
    env->CallStaticVoidMethod(g_class, method);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}

bool native_ask_result(std::string *text, bool *accepted)
{
    std::lock_guard<std::mutex> lock(g_mu);
    if (g_results.empty()) {
        return false;
    }
    if (text) {
        *text = g_results.front().first;
    }
    if (accepted) {
        *accepted = g_results.front().second;
    }
    g_results.pop_front();
    return true;
}

}  // namespace cb

#else  // десктоп: настоящих окон нет, игра рисует всё сама

namespace cb {

bool native_ui_available()
{
    return false;
}

void native_toast(const std::string &)
{
}

void native_ask(const std::string &, const std::string &, const std::string &,
                const std::string &, const std::string &)
{
}

void native_ask_dismiss()
{
}

bool native_ask_result(std::string *, bool *)
{
    return false;
}

}  // namespace cb

#endif  // __ANDROID__ && CB_USE_SDL2
