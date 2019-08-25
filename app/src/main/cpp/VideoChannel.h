//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_VIDEOCHANNLE_H
#define HAWKEYEPLAYER_VIDEOCHANNLE_H


#include "BaseChannel.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
};

typedef void (*RenderCallback)(uint8_t *data, int lineSize, int Width, int height);

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int id, AVCodecContext *codecCtx, int fps);

    ~VideoChannel();

    void start();

    void stop();

    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback renderCallback);

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback renderCallback;
    int fps;
};


#endif //HAWKEYEPLAYER_VIDEOCHANNLE_H
