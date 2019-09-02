//
// Created by lixiaoxu on 2019-08-14.
//



#include "VideoChannel.h"

/**
 * 丢包(AVPAcket)
 * @param q
 */
void dropAVPacket(queue<AVPacket *> &q) {
    if (!q.empty()) {
        AVPacket *avPacket = q.front();
        //I帧， B帧，P帧
        //不能丢I帧
        if (avPacket->flags != AV_PKT_FLAG_KEY) {
            //丢弃非I帧
            BaseChannel::releaseAVPacket(&avPacket);
            LOGI("丢弃未解码一帧,packet");
            q.pop();
        }
    }
}

/**
 * 丢包(AVPAcket)
 * @param q
 */
void dropAVFrame(queue<AVFrame *> &q) {
    if (!q.empty()) {
        AVFrame *avFrame = q.front();
        BaseChannel::releaseAVFrame(&avFrame);
        LOGI("丢弃画面一帧,frame");
        q.pop();
    }
}


VideoChannel::VideoChannel(int id, AVCodecContext *codecCtx, int fps, AVRational time_base)
        : BaseChannel(id, codecCtx, time_base) {
    this->fps = fps;
    packets.setSyncHandle(dropAVPacket);
    frames.setSyncHandle(dropAVFrame);
}

VideoChannel::~VideoChannel() {

}

void *task_video_decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();
    return 0;
}


void *task_video_play(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();
    return 0;
}


void VideoChannel::start() {
    isPlaying = 1;
    //设置队列工作状态
    packets.setWork(1);
    frames.setWork(1);
    //解码
    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    //播放
    pthread_create(&pid_video_play, 0, task_video_play, this);

}

void VideoChannel::stop() {
    isPlaying = 0;
}

/**
 * 真正视频解码
 */
void VideoChannel::video_decode() {
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
        frames.push(frame);

    }
    releaseAVPacket(&packet);

}

void VideoChannel::video_play() {
    AVFrame *frame = 0;
    int ret = 0;
    //对原始数据进行转换：yuv > rgba

    //struct SwsContext *sws_getContext(int srcW, int srcH, enum AVPixelFormat srcFormat,
    //                                  int dstW, int dstH, enum AVPixelFormat dstFormat,
    //                                  int flags, SwsFilter *srcFilter,
    //                                  SwsFilter *dstFilter, const double *param);
    SwsContext *swsCtx = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
                                        codecCtx->width, codecCtx->height, AV_PIX_FMT_RGBA,
                                        SWS_BILINEAR,
                                        NULL, NULL, NULL);
    uint8_t *dst_data[4];
    int dst_linesize[4];
    //int av_image_alloc(uint8_t *pointers[4], int linesizes[4],
    //                   int w, int h, enum AVPixelFormat pix_fmt, int align);
    ret = av_image_alloc(dst_data, dst_linesize,
                         codecCtx->width, codecCtx->height, AV_PIX_FMT_RGBA, 1);
    //根据fps控制每一帧的延时时间
    //sleep:fps 转成 时间，
    //单位是秒
    double delay_time_per_frame = 1.0 / fps;
    while (isPlaying) {
        ret = frames.pop(frame);
        if (!isPlaying) {
            //如果停止播放了，跳出循环
            break;
        }
        if (!ret) {
            continue;
        }
        //取到了yuv原始数据，进行转换
        sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height, dst_data,
                  dst_linesize);
        //进行休眠
        //每一帧还有自己额外延时时间
        double extra_delay = frame->repeat_pict / (2 * fps);
        double real_delay = extra_delay + delay_time_per_frame;
        //单位是：微妙
//        av_usleep(real_delay * 1000000);
        //需要使用音频时间来判断
        //获取视频的播放时间
        double video_time = frame->best_effort_timestamp * av_q2d(time_base);
        if (!audioChannel) {
            LOGI("没有音频，不需要延迟");
            //没有音频
            av_usleep(real_delay * 1000000);
        } else {
            double audioTime = audioChannel->audio_time;
            double time_diff = video_time - audioTime;
            LOGI("时间差值time_diff: %f", time_diff);
            if (time_diff > 0) {
                //视频比音频快，sleep
                //判断time_diff值大小，seek后time_diff有可能会很大,导致休眠太久
                if (time_diff > 1) {
                    //等音频慢慢赶上
                    av_usleep(real_delay * 1000000);
                } else {
                    av_usleep((real_delay + time_diff) * 1000000);
                }
            } else if (time_diff < 0) {
                //音频比视频快,尝试丢包
                //视频包：编码前packets，编码后frames
                if (fabs(time_diff) >= 0.05) {
                    //时间差如果大于0.05,有明显的延迟感
                    //丢包：操作队列中数据！一定要小心
                    packets.sync();
                    //frames.sync();
                    continue;
                }
            }

        }


        //dst_data:AV_PIX_FMT_RGBA格式的数据
        //进行渲染，回调出去 native-lib
        //渲染图像，需要什么信息？
        //宽高，尺寸
        //内容，数据，图像画法
        //1Data,2.linesize,3.width,4.height

        renderCallback(dst_data[0], dst_linesize[0], codecCtx->width, codecCtx->height);
        //frame释放
        releaseAVFrame(&frame);
        frame = 0;
    }
    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(swsCtx);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}
