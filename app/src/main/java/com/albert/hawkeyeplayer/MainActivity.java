package com.albert.hawkeyeplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.Toast;

import com.albert.hawkeyeplayer.View.EyePlayer;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {

    private static final String TAG = "MainActivity";
    private SurfaceView surfaceView;
    private EyePlayer player;
    private SeekBar seekBar;
    private boolean isTouch = false;
    private int seekProgress = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        requestPermissions();
        surfaceView = findViewById(R.id.surface_view);
        seekBar = (SeekBar) findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(this);
        player = new EyePlayer();
        player.setSurfaceView(surfaceView);

        player.setDataSource(new File(Environment.getExternalStorageDirectory()
                + File.separator + "input.mp4").getAbsolutePath());
        player.setOnErrorListener(new EyePlayer.OnErrorListener() {
            @Override
            public void onError(final int errorCode) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.e(TAG, "出现异常，错误码" + errorCode);
                    }
                });
            }
        });
        player.setOnPreparedListener(new EyePlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                final int duration = player.getDuration();
                if (duration != 0) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            seekBar.setVisibility(View.VISIBLE);
                        }
                    });
                }
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.e(TAG, "开始播放  ");
                        //准备完成
                        Toast.makeText(MainActivity.this, "开始播放", Toast.LENGTH_SHORT).show();
                    }
                });
                //播放 调用到native去
                //开始播放
//                player.start();
            }
        });

        player.setOnProgressListener(new EyePlayer.OnProgressListener() {
            @Override
            public void onProgress(final float time) {
                if (isTouch) {
                    return;
                }
                seekBar.setProgress((int) (time / player.getDuration() * seekBar.getMax()));
            }
        });

    }

    private void requestPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            String[] perms = {Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WAKE_LOCK};
            if (checkSelfPermission(
                    perms[0]) == PackageManager.PERMISSION_DENIED || checkSelfPermission(
                    perms[1]) == PackageManager.PERMISSION_DENIED) {
                requestPermissions(perms, 200);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
//        player.prepare();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.release();
    }

    public void Prepare(View view) {
        player.prepare();
    }

    public void Play(View view) {
        player.start();
    }

    public void Pause(View view) {
    }

    public void Stop(View view) {
        //TODO 停止并释放资源
        player.stop();

    }

    /**
     * 跟随进度自动更新,拿到总时长
     * 1.总时长
     * 2.当前播放时间
     *
     * @param seekBar
     * @param progress
     * @param fromUser
     */
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
//        Log.i(TAG, "onProgressChanged: " + fromUser + "~" + progress + "~" + isTouch);
//        if (fromUser && !isTouch) {
            seekBar.setProgress(progress);
//        }

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        //获取seekBar当前进度,进度条转换为真实时间
        double playProgress = seekBar.getProgress() * 1.0 / seekBar.getMax() * player.getDuration();
        player.seekTo(playProgress);
        seekProgress = seekBar.getProgress();
        isTouch = false;

    }
}
