//
// Created by lixiaoxu on 2019-08-14.
//

#include "JavaCallHelper.h"

JavaCallHelper::JavaCallHelper(JavaVM *javaVM_, JNIEnv *env_, jobject instance_) {
    this->javaVM = javaVM_;
    this->env = env_;
    //不能直接赋值
//    this->instance = instance_;
    //一旦涉及到jobject跨方法和线程，都需要创建全局引用
    this->instance = env->NewWeakGlobalRef(instance_);
    this->clazz = env->GetObjectClass(instance);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
    jmd_error = env->GetMethodID(clazz, "onError", "(I)V");
    jmd_progress = env->GetMethodID(clazz, "onProgress", "(F)V");


}

JavaCallHelper::~JavaCallHelper() {
    javaVM = 0;
    env->DeleteGlobalRef(instance);
    instance = 0;
}

void JavaCallHelper::onPrepared(int threadMode) {
    if (threadMode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_prepared);
    }

    if (threadMode == THREAD_CHILD) {
        //子线程
        //当前子线程的Env
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_prepared);
        javaVM->DetachCurrentThread();
    }
}

void JavaCallHelper::onError(int threadMode, int errorCode) {
    if (threadMode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_error, errorCode);
    }

    if (threadMode == THREAD_CHILD) {
        //子线程
        //当前子线程的Env
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_error, errorCode);
        javaVM->DetachCurrentThread();

    }

}

void JavaCallHelper::onProgress(int threadMode, float time) {
    if (threadMode == THREAD_MAIN) {
        env->CallVoidMethod(instance, jmd_progress, time);
    }
    if (threadMode == THREAD_CHILD) {
        //子线程
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_progress, time);
        javaVM->DetachCurrentThread();
    }
}

