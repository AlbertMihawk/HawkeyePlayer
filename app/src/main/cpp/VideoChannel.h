//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_VIDEOCHANNLE_H
#define HAWKEYEPLAYER_VIDEOCHANNLE_H


#include "BaseChannel.h"
#include "AudioChannel.h"
#include "JavaCallHelper.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

typedef void (*RenderCallback)(uint8_t *data, int lineSize, int Width, int height);

class VideoChannel : public BaseChannel {
public:
    VideoChannel(JavaCallHelper *javaCallHelper,int id, AVCodecContext *codecCtx, int fps, AVRational time_base);

    ~VideoChannel();

    void start();

    void stop();

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback renderCallback);

    void setAudioChannel(AudioChannel *audioChannel);

private:
    JavaCallHelper *javaCallHelper;
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback renderCallback;
    int fps;
    AudioChannel *audioChannel = 0;
};


#endif //HAWKEYEPLAYER_VIDEOCHANNLE_H
