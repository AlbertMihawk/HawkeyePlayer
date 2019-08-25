//
// Created by lixiaoxu on 2019-08-14.
//


#include "VideoChannel.h"

VideoChannel::VideoChannel(int id, AVCodecContext *codecCtx) : BaseChannel(id, codecCtx) {

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
        //dst_data:AV_PIX_FMT_RGBA格式的数据
        //进行渲染，回调出去 native-lib
        //渲染图像，需要什么信息？
        //宽高，尺寸
        //内容，数据，图像画法
        //1Data,2.linesize,3.width,4.height

        renderCallback(dst_data[0], dst_linesize[0], codecCtx->width, codecCtx->height);
        //frame释放
        releaseAVFrame(&frame);
    }
    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(swsCtx);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}
