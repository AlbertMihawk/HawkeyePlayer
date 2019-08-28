//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_AUDIOCHANNEL_H
#define HAWKEYEPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, AVCodecContext *codecCtx);

    ~AudioChannel();

    void start();

    void stop();

    void audio_decode();

    void audio_play();

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    int fps;
};


#endif //HAWKEYEPLAYER_AUDIOCHANNEL_H
