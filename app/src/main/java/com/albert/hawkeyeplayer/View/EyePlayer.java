package com.albert.hawkeyeplayer.View;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class EyePlayer implements SurfaceHolder.Callback {

    static {

        System.loadLibrary("hawkeyeplayer");
    }

    //直播地址或者媒体文件路径
    private String dataSource;
    private SurfaceHolder mHolder;

    private OnErrorListener onErrorListener;
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
     * 给jni回调用错误
     * errorCode 从JNI通过反射传递过来
     *
     * @param errorCode
     */
    public void onError(int errorCode) {
        if (onErrorListener != null) {
            onErrorListener.onError(errorCode);
        }
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

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    private native void prepareNative(String dataSource);

    public void setSurfaceView(SurfaceView surfaceView) {
        if (mHolder != null) {
            mHolder.removeCallback(this);
        }
        mHolder = surfaceView.getHolder();
        mHolder.addCallback(this);
    }


    /**
     * 创建画布
     *
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布刷新
     *
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(holder.getSurface());
    }

    /**
     * 画布销毁
     *
     * @param holder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public native void setSurfaceNative(Surface surface);


    /**
     * 资源释放
     */
    public void release() {
        mHolder.removeCallback(this);
        releaseNative();
    }

    private native void releaseNative();

    /**
     * 停止播放
     */
    public void stop() {
        stopNative();
    }

    private native void stopNative();

    public interface OnPreparedListener {
        void onPrepared();
    }

    public interface OnErrorListener {
        void onError(int errorCode);
    }
}
