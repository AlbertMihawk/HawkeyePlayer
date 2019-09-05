//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_EYEFFMPEG_H
#define HAWKEYEPLAYER_EYEFFMPEG_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include <cstring>
#include "marco.h"
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
};

class EyeFFmpeg {
    //使用友元函数
    friend void *task_stop(void *args);

public:
    EyeFFmpeg(JavaCallHelper *javaCallHelper, char *dataSource);

    ~EyeFFmpeg();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void setRenderCallback(RenderCallback renderCallback);

    void stop();

    void _stop();

    int getDuration() const;

    int seekTo(jdouble d);

private:
    JavaCallHelper *javaCallHelper = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_start;
    pthread_t pid_stop;
    bool isPreparing;
    RenderCallback renderCallback;

    AVFormatContext *formatCtx;
    int duration;//总播放时长
};


#endif //HAWKEYEPLAYER_EYEFFMPEG_H
