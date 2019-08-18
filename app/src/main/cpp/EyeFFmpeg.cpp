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

/**
 * 准备线程执行任务
 * @param args
 * @return
 */
void *task_prepare(void *args) {

    //打开输入
    //args参数是this
    EyeFFmpeg *fFmpeg = static_cast<EyeFFmpeg *>(args);

    //datasource
    fFmpeg->_prepare();
//    转移到成员函数avformat_open_input(&formatCtx, fFmpeg->dataSource, 0, 0);

    return 0;//函数指针一定一定要返回0
}

void EyeFFmpeg::_prepare() {
    //1 ffmpegcontext

    AVFormatContext *formatCtx = avformat_alloc_context();//创建上下文
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "10000000", 0);
    int ret = avformat_open_input(&formatCtx, dataSource, 0, &dictionary);
    //需要释放
    av_dict_free(&dictionary);
    if (ret) {
        //失败，回调给Java层
        LOGE("avformat 打开媒体失败 %s", av_err2str(ret));
        //JavaCallHelper jni 回调Java
        // JavaCallHelper.onError(ret);
        //Java层需要根据errorCode来更新ui
    }


}

/**
 * 播放准备
 * 可能是主线程
 * doc/samples
 * @return
 */
void EyeFFmpeg::prepare() {
    //解封装
    //可以直接来进行解码api调用么？
    //不能直接执行
    //文件：IO流
    //直播：网络流

    //创建子线程
    // pthread_create(pthread_t* __pthread_ptr, 线程指针
    // pthread_attr_t const* __attr, 参数
    // void* (*__start_routine)(void*), 执行的函数指针
    // void*); 回调函数的参数
    pthread_create(&pid_prepare, 0, task_prepare, this);


}


