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
import android.widget.Toast;

import com.albert.hawkeyeplayer.View.EyePlayer;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private SurfaceView surfaceView;
    private EyePlayer player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        requestPermissions();
        surfaceView = findViewById(R.id.surface_view);
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

    public void Prepare(View view) {
        player.prepare();
    }

    public void Play(View view) {
        player.start();
    }
}
