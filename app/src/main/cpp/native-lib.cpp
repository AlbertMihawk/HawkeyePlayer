#include <jni.h>
#include <string>
#include "EyeFFmpeg.h"

extern "C" {
#include <libavutil/avutil.h>
}

JavaVM *javaVM = 0;
JavaCallHelper *javaCallHelper = 0;
EyeFFmpeg *ffmpeg = 0;

extern "C" JNIEXPORT jstring JNICALL
Java_com_albert_hawkeyeplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}

jint JNI_OnLoad(JavaVM *vm, void *reserved){
    javaVM = vm;
    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_albert_hawkeyeplayer_View_EyePlayer_prepareNative(JNIEnv *env, jobject instance,
                                                           jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);

    javaCallHelper = new JavaCallHelper(javaVM, env, instance);
    ffmpeg = new EyeFFmpeg(javaCallHelper, const_cast<char *>(dataSource));
    ffmpeg->prepare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_albert_hawkeyeplayer_View_EyePlayer_startNative(JNIEnv *env, jobject instance) {

    ffmpeg->start();
}