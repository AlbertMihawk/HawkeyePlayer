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

private:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject instance;
    jmethodID jmd_prepared;

};


#endif //HAWKEYEPLAYER_JAVACALLHELPER_H
