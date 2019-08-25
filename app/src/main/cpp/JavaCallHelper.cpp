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
    jclass clazz = env->GetObjectClass(instance);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");


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
