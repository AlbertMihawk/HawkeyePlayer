#include <jni.h>
#include <string>
#include "EyeFFmpeg.h"

extern "C" {
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_albert_hawkeyeplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(av_version_info());
}


extern "C"
JNIEXPORT void JNICALL
Java_com_albert_hawkeyeplayer_View_EyePlayer_prepareNative(JNIEnv *env, jobject instance,
                                                           jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);

    JavaCallHelper *javaCallHelper = new JavaCallHelper();
    EyeFFmpeg *ffmpeg = new EyeFFmpeg(javaCallHelper, const_cast<char *>(dataSource));
    ffmpeg->prepare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}