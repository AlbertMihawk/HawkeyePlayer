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

}

//内存释放
EyeFFmpeg::~EyeFFmpeg() {
    DELETE(dataSource);
    DELETE(javaCallHelper);

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
        //判断流类型（Audio，Video）
        AVMediaType mediaType = codecParams->codec_type;
        //如果是音频
        if (mediaType == AVMEDIA_TYPE_AUDIO) {
            //AudioChannel
            audioChannel = new AudioChannel(i, codecContext);
        }
        //如果是视频
        if (mediaType == AVMEDIA_TYPE_VIDEO) {
            //VideoChannel
            videoChannel = new VideoChannel(i, codecContext);
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
    videoChannel->start();
    audioChannel->start();


    pthread_create(&pid_start, 0, task_start, this);

}

/**
 * 子线程播放操作
 */
void EyeFFmpeg::_start() {
    while (isPreparing) {
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(formatCtx, packet);
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
            //TODO

        } else {
            //TODO 作业，出现错误
            LOGE("读取音视频错误");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, ret);
            }
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


