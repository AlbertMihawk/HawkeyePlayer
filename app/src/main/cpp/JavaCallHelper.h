//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_JAVACALLHELPER_H
#define HAWKEYEPLAYER_JAVACALLHELPER_H

#include <jni.h>
#include "marco.h"

class JavaCallHelper {

public:
    JavaCallHelper(JavaVM *javaVM, JNIEnv *env, jobject instance);

    ~JavaCallHelper();

    void onPrepared(int threadMode);

    void onError(int threadMode, int errorCode);

    void onProgress(int threadMode, float time);

private:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject instance;
    jclass clazz;
    jmethodID jmd_prepared;
    jmethodID jmd_error;
    jmethodID jmd_progress;

};


#endif //HAWKEYEPLAYER_JAVACALLHELPER_H
