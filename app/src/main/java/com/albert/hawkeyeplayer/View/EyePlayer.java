package com.albert.hawkeyeplayer.View;

public class EyePlayer {

    static {

        System.loadLibrary("hawkeyeplayer");
    }

    //直播地址或者媒体文件路径
    private String dataSource;
    private OnErrorListener errorlistener;
    private OnPreparedListener onPreparedListener;

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 播放准备工作
     */
    public void prepare() {
        prepareNative(this.dataSource);
    }

    /**
     * 开始播放
     */
    public void start() {
        startNative();
    }

    //可以尝试使用动态注册
    public native void startNative();

    /**
     * 给jni回调用
     * errorCode 从JNI通过反射传递过来
     *
     * @param errorCode
     */
    public void onError(int errorCode) {
        errorlistener.onError(errorCode);

    }

    /**
     * 供native反射调用
     * 表示发播放器准备好了，可以播放了
     */
    public void onPrepared() {
        if (onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    public void setListener(OnErrorListener errorlistener) {
        this.errorlistener = errorlistener;
    }

    private native void prepareNative(String dataSource);

    public interface OnPreparedListener {
        void onPrepared();
    }


    public interface OnErrorListener {
        void onError(int errorCode);
    }
}
