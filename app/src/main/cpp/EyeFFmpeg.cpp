//
// Created by lixiaoxu on 2019-08-14.
//


#include "EyeFFmpeg.h"


EyeFFmpeg::EyeFFmpeg(JavaCallHelper *javaCallHelper, char *dataSource) {
    this->javaCallHelper = javaCallHelper;
    /**
     * dataSource通过JNI从JAVA传过来的字符串，转换成c++字符串
     * 在jni被释放掉了，导致dataSource指针变成悬空指针。
     * jni的方法必须对内存进行释放
     * 通过内存拷贝， 自己解决问题
     * strlen() 获取字符串长度，
     * strcpy() 字符串拷贝
     */

    //c 字符串以\0结尾，java缺少1个字符
    this->dataSource = new char[strlen(dataSource) + 1];
    //完成字符串拷贝
    strcpy(this->dataSource, dataSource);

}

//内存释放
EyeFFmpeg::~EyeFFmpeg() {
    DELETE(dataSource);
    DELETE(javaCallHelper);

}

//播放准备
void EyeFFmpeg::prepare() {
    //解封装
    //可以直接来进行解码api调用么？
    90：42

}
