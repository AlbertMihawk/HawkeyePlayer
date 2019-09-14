//
// Created by lixiaoxu on 2019-08-14.
//




#include "EyeFFmpeg.h"


EyeFFmpeg::EyeFFmpeg(JavaCallHelper *javaCallHelper, char *dataSource) {
    this->javaCallHelper = javaCallHelper;
    /**
     * dataSource通过JNI从JAVA传过来的字符串，转换成c++字符串
     * 在jni被释放掉了，导致dataSource指针变成悬空指针。
     * jni的方法必须对内存进行释放
     * 通过内存拷贝， 自己解决问题
     * strlen() 获取字符串长度，
     * strcpy() 字符串拷贝
     */

    //c 字符串以\0结尾，java缺少1个字符
    this->dataSource = new char[strlen(dataSource) + 1];
    //完成字符串拷贝
    strcpy(this->dataSource, dataSource);
    //formatContext加锁
    pthread_mutex_init(&seekMutex, 0);

}

//内存释放
EyeFFmpeg::~EyeFFmpeg() {
    DELETE(dataSource);
    DELETE(javaCallHelper);
    pthread_mutex_destroy(&seekMutex);
}

/**
 * 准备线程执行任务
 * @param args
 * @return
 */
void *task_prepare(void *args) {

    //打开输入
    //args参数是this
    EyeFFmpeg *fFmpeg = static_cast<EyeFFmpeg *>(args);

    //datasource
    fFmpeg->_prepare();
//    转移到成员函数avformat_open_input(&formatCtx, fFmpeg->dataSource, 0, 0);

    return 0;//函数指针一定一定要返回0
}

/**
 * 子线程释放内存
 * @param args
 * @return
 */
void *task_stop(void *args) {

    EyeFFmpeg *ffmpeg = static_cast<EyeFFmpeg *>(args);

    ffmpeg->isPreparing = 0;
    //添加子线程执行顺序到主线程
    pthread_join(ffmpeg->pid_prepare, 0);

    //在主线程，要保证子线程中_prepare方法执行完
    if (ffmpeg->formatCtx) {
        avformat_close_input(&ffmpeg->formatCtx);
        avformat_free_context(ffmpeg->formatCtx);
        ffmpeg->formatCtx = 0;
    }


    if (ffmpeg->videoChannel) {
        ffmpeg->videoChannel->stop();
    }
    if (ffmpeg->audioChannel) {
        ffmpeg->audioChannel->stop();
    }

    DELETE(ffmpeg->videoChannel)
    DELETE(ffmpeg->audioChannel)
    DELETE(ffmpeg)
    return 0;
}


void EyeFFmpeg::_prepare() {
    //1 ffmpegcontext

    formatCtx = avformat_alloc_context();//创建上下文
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "10000000", 0);
    //1.打开媒体文件
    int ret = avformat_open_input(&formatCtx, dataSource, 0, &dictionary);
    //需要释放
    av_dict_free(&dictionary);
    if (ret) {
        //失败，回调给Java层
        LOGE("avformat 打开媒体失败 %s", av_err2str(ret));
        //JavaCallHelper jni 回调Java
        //TODO 自己实现异常回调
        // JavaCallHelper.onError(ret);
        //Java层需要根据errorCode来更新ui
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, ret);
        }
        return;
    }
    //2.查找媒体中的编码方式(流信息)
    ret = avformat_find_stream_info(formatCtx, 0);
    if (ret < 0) {
        //TODO 作业
        LOGE("查找媒体编码方式失败");
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, ret);
        }
        return;
    }
    //上下文拿到总时长
    duration = formatCtx->duration / AV_TIME_BASE;
    LOGI("总时长 %ld", duration);
    for (int i = 0; i < formatCtx->nb_streams; ++i) {
        AVStream *stream = formatCtx->streams[i];
        //获取编解码流参数
        AVCodecParameters *codecParams = stream->codecpar;
        //3.通过id拿到编解码器
        AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
        if (!codec) {
            //TODO 作业
            LOGE("编码器创建失败");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, ret);
            }
            return;
        }
        //4.获得编解码器的上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        //5.设置解码器上下文参数
        ret = avcodec_parameters_to_context(codecContext, codecParams);
        if (ret < 0) {
            //TODO 作业
            LOGE("查找媒体编码方式失败");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, ret);
            }
            return;
        }
        //6.打开解码器
        ret = avcodec_open2(codecContext, codec, 0);
        if (ret < 0) {
            //TODO 作业
            LOGE("打开解码器失败");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, ret);
            }
            return;
        }

        AVRational time_base = stream->time_base;

        //判断流类型（Audio，Video）
        AVMediaType mediaType = codecParams->codec_type;
        //如果是音频
        if (mediaType == AVMEDIA_TYPE_AUDIO) {
            //AudioChannel
            audioChannel = new AudioChannel(i, codecContext, time_base);
        }
        //如果是视频
        if (mediaType == AVMEDIA_TYPE_VIDEO) {
            //VideoChannel
            AVRational avRational = stream->avg_frame_rate;
            //fps帧率
//            int fps = avRational.num / avRational.den;
            double fps = av_q2d(avRational);
            videoChannel = new VideoChannel(javaCallHelper, i, codecContext, fps, time_base);
            videoChannel->setRenderCallback(renderCallback);
        }
    }

    if (!audioChannel && !videoChannel) {
        //既没有音频，有没有视频
        //TODO 作业
        LOGE("音频和视频都没有找到");
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, ret);
        }
        return;
    }

    //准备好了,反射通知Java
    if (javaCallHelper) {
        javaCallHelper->onPrepared(THREAD_CHILD);
    }
}


/**
 * 播放准备
 * 可能是主线程
 * doc/samples
 * @return
 */
void EyeFFmpeg::prepare() {
    //解封装
    //可以直接来进行解码api调用么？
    //不能直接执行
    //文件：IO流
    //直播：网络流

    //创建子线程
    // pthread_create(pthread_t* __pthread_ptr, 线程指针
    // pthread_attr_t const* __attr, 参数
    // void* (*__start_routine)(void*), 执行的函数指针
    // void*); 回调函数的参数
    pthread_create(&pid_prepare, 0, task_prepare, this);


}

void *task_start(void *args) {
    EyeFFmpeg *ffmpeg = static_cast<EyeFFmpeg *>(args);

    ffmpeg->_start();
    return 0;

}

/**
 * 开始播放
 */
void EyeFFmpeg::start() {
    isPreparing = 1;
    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->start();
    }
    if (audioChannel) {
        audioChannel->start();
    }

    pthread_create(&pid_start, 0, task_start, this);

}

/**
 * 子线程播放操作
 */
void EyeFFmpeg::_start() {

    while (isPreparing) {
        /**
         * 内存泄露点1
         * 控制packets队列数量
           */
        if (videoChannel && videoChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        /**
         * 泄露点2
         * 控制音频packet队列数量
         */
        if (audioChannel && audioChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        pthread_mutex_lock(&seekMutex);
        int ret = av_read_frame(formatCtx, packet);
        pthread_mutex_unlock(&seekMutex);
        if (!ret) {
            //ret 为0是正确
            //区分流类型，音频还是视频
            if (videoChannel && packet->stream_index == videoChannel->id) {
                //添加视频数据到数据队列
                videoChannel->packets.push(packet);
            } else if (audioChannel && packet->stream_index == audioChannel->id) {
                //添加音频数据到数据队列
                audioChannel->packets.push(packet);
            }

        } else if (ret == AVERROR_EOF) {
            //ret不为0，end of数据到结尾,读完了
            //有可能读完了，播放为完成
            //判断有没有播放完
            if (videoChannel->packets.empty() && videoChannel->frames.empty()) {
                //播放完成
                av_packet_free(&packet);
                break;
            }

        } else {
            //TODO 作业，出现错误
            LOGE("读取音视频错误");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, ret);
            }
            av_packet_free(&packet);
            break;
        }

    }
    isPreparing = 0;

    //停止音视频解码
    videoChannel->stop();
    audioChannel->stop();

}

void EyeFFmpeg::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void EyeFFmpeg::stop() {
//    isPreparing = 0;
    //prepare阻塞中停止了，不回调java
    javaCallHelper = 0;
    //既然在主线程引发ANR，那么到子线程去释放
    pthread_create(&pid_stop, 0, task_stop, this);

//    //在主线程，要保证子线程中_prepare方法执行完
//    if (formatCtx) {
//        avformat_close_input(&formatCtx);
//        avformat_free_context(formatCtx);
//        formatCtx = 0;
//    }
//    if (videoChannel) {
//        videoChannel->stop();
//    }
//    if (audioChannel) {
//        audioChannel->stop();
//    }

}

int EyeFFmpeg::getDuration() const {
    return duration;
}

/**
 * return 为0为成功seek
 * @param d
 * @return
 */
int EyeFFmpeg::seekTo(double d) {
    int result = 0;
    if (d < 0 && d > duration) {
        result = -1;
    }


    if (!formatCtx || !audioChannel || !videoChannel) {
        return -1;
    }

    pthread_mutex_lock(&seekMutex);
    //1.上下文
    //2.流索引，-1，表示默认流
    //3.时间戳
    //4.seek方式
    //AVSEEK_FLAG_BACKWARD：表示seek到请求时间错之前的关键帧
    //AVSEEK_FLAG_BYTE：基于字节位置
    //AVSEEK_FLAG_ANY：任意一帧（可能不是关键帧，会花屏）
    //AVSEEK_FLAG_FRAME：基于帧数seek
    int ret = av_seek_frame(formatCtx, -1, d * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, ret);
        }
        return -1;
    }

    if (audioChannel) {
        audioChannel->packets.setWork(0);
        audioChannel->frames.setWork(0);
        audioChannel->packets.clear();
        audioChannel->frames.clear();
        //清除数据后，让队列重新工作
        audioChannel->packets.setWork(1);
        audioChannel->frames.setWork(1);
    }

    if (videoChannel) {
        videoChannel->packets.setWork(0);
        videoChannel->frames.setWork(0);
        videoChannel->packets.clear();
        videoChannel->frames.clear();
        //清除数据后，让队列重新工作
        videoChannel->packets.setWork(1);
        videoChannel->frames.setWork(1);
    }

    pthread_mutex_unlock(&seekMutex);
    return result;
}


