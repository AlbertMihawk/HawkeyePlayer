//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_AUDIOCHANNEL_H
#define HAWKEYEPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, AVCodecContext *codecCtx);

     ~AudioChannel();

    void start();

    void stop();

};


#endif //HAWKEYEPLAYER_AUDIOCHANNEL_H
