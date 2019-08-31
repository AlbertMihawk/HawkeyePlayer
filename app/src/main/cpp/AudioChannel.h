//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_AUDIOCHANNEL_H
#define HAWKEYEPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>
#include "marco.h"

extern "C" {
#include <libswresample/swresample.h>
};

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, AVCodecContext *codecCtx, AVRational time_base);

    ~AudioChannel();

    void start();

    void stop();

    void audio_decode();

    void audio_play();

    int getPCM();

    uint8_t *out_buffers;
    int out_channels;
    int out_sampleSize;
    int out_sampleRate;
    int out_bufferSize;

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    int fps;


    SwrContext *swrCtx = 0;
    //引擎
    SLObjectItf engineObject = 0;
    //引擎接口
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerPlay = 0;
    //播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

};


#endif //HAWKEYEPLAYER_AUDIOCHANNEL_H
