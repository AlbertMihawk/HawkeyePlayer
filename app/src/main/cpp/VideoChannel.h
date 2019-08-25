//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_VIDEOCHANNLE_H
#define HAWKEYEPLAYER_VIDEOCHANNLE_H


#include "BaseChannel.h"
#include "safe_queue.h"

class VideoChannel : public BaseChannel {
public:
    VideoChannel(int id);

    ~VideoChannel();

    void start();

    void stop();
};


#endif //HAWKEYEPLAYER_VIDEOCHANNLE_H
