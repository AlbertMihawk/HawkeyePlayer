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
public:
    EyeFFmpeg(JavaCallHelper *javaCallHelper, char *dataSource);

    ~EyeFFmpeg();

    void prepare();

    void _prepare();

private:
    JavaCallHelper *javaCallHelper = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    pthread_t pid_prepare;
    char *dataSource;
};


#endif //HAWKEYEPLAYER_EYEFFMPEG_H
