//
// Created by lixiaoxu on 2019-08-14.
//

#include "AudioChannel.h"


AudioChannel::AudioChannel(int id, AVCodecContext *codecCtx) : BaseChannel(id, codecCtx) {
}

AudioChannel::~AudioChannel() {

}


void *task_audio_decode(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->audio_decode();
    return 0;
}


void *task_audio_play(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->audio_play();
    return 0;
}

void AudioChannel::start() {
    isPlaying = 1;
    //设置队列工作状态
    packets.setWork(1);
    frames.setWork(1);
    //解码
    pthread_create(&pid_audio_decode, 0, task_audio_decode, this);
    //播放
    pthread_create(&pid_audio_play, 0, task_audio_play, this);

}

void AudioChannel::stop() {

}

/**
 * 音频解码
 */
void AudioChannel::audio_decode() {
    int ret = 0;
    AVPacket *packet = 0;
    while (isPlaying) {
        ret = packets.pop(packet);
        if (!isPlaying) {
            //如果停止播放了，跳出循环，释放packet
            break;
        }
        if (!ret) {
            continue;
        }
        //拿到了视频数据包（编码压缩了），需要把数据包给解码器

        ret = avcodec_send_packet(codecCtx, packet);
        if (ret) {
            //往解码器发送数据失败,跳出循环
            break;
        }
        releaseAVPacket(&packet);
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecCtx, frame);
        if (ret == AVERROR(EAGAIN)) {
            //重试
            continue;
        } else if (ret != 0) {
            break;
        }
        //成功获取解码后数据包
        //把frame存到队列，进入播放
        /**
         * 内存泄露点2
         * 控制frames队列数量
         */
        while (isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        frames.push(frame);//PCM数据队列
    }
    releaseAVPacket(&packet);

}

//4.3创建回调函数
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);

    77:00
    (*bq)->Enqueue(bq, nextBuffer, nextSize);
}


/**
 * 音频播放
 */
void AudioChannel::audio_play() {
    //音频解码7部曲
    //1创建引擎对象和接口
    //1.1创建引擎对象：SLObjectItf engineObject

    //1.2 初始化引擎

    //1.3获取引擎接口 SLEngineItf engineInterface



}
