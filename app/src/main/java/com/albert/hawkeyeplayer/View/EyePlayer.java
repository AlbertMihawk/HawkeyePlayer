package com.albert.hawkeyeplayer.View;

public class EyePlayer {

    static {

        System.loadLibrary("native-lib");
    }

    //直播地址或者媒体文件路径
    private String dataSource;

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    /**
     * 播放准备工作
     */
    public void prepare() {
        prepareNative(this.dataSource);
    }

    private native void prepareNative(String dataSource);
}
