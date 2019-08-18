package com.albert.hawkeyeplayer.View;

public class EyePlayer {

    static {

        System.loadLibrary("hawkeyeplayer");
    }

    //直播地址或者媒体文件路径
    private String dataSource;
    private MyErrorListener listener;

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
     * 给jni回调用
     * errorCode 从JNI通过反射传递过来
     *
     * @param errorCode
     */
    public void onError(int errorCode) {
        listener.onError(errorCode);

    }

    public void setListener(MyErrorListener listener) {
        this.listener = listener;
    }

    private native void prepareNative(String dataSource);

    public interface MyErrorListener {
        void onError(int errorCode);
    }
}
