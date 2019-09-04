//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_BASECHANNEL_H
#define HAWKEYEPLAYER_BASECHANNEL_H

#include "safe_queue.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

/**
 * VideoChannel 和AudilChannel的父类
 */

class BaseChannel {
public:
    BaseChannel(int id, AVCodecContext *codecCtx, AVRational time_base) :
            id(id), codecCtx(codecCtx), time_base(time_base) {
        packets.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    //虚函数析构函数，需要子类实现的
    //不然释放的时候会有问题
    virtual ~BaseChannel() {

        packets.clear();
        frames.clear();
        if (codecCtx) {
            avcodec_close(codecCtx);
            avcodec_free_context(&codecCtx);
            codecCtx = 0;
        }
    }

    static void releaseAVPacket(AVPacket **packet) {
        if (packet) {
            av_packet_free(packet);
            *packet = 0;
        }
    }

    static void releaseAVFrame(AVFrame **frame) {
        if (frame) {
            av_frame_free(frame);
            *frame = 0;
        }
    }

    //纯虚函数（抽象方法）
    virtual void start() = 0;

    virtual void stop() = 0;


    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    int id;
    bool isPlaying = 0;
    //解码器上下文
    AVCodecContext *codecCtx;
    AVRational time_base;
    double audio_time;
};

#endif //HAWKEYEPLAYER_BASECHANNEL_H
