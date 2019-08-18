package com.albert.hawkeyeplayer;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.WindowManager;

import com.albert.hawkeyeplayer.View.EyePlayer;

import java.io.File;

public class MainActivity extends AppCompatActivity {


    private SurfaceView surfaceView;
    private EyePlayer player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        surfaceView = findViewById(R.id.surface_view);
        player = new EyePlayer();
        player.setDataSource(new File(Environment.getExternalStorageDirectory()
                + File.separator + "input.mp4").getAbsolutePath());
        player.setListener(new EyePlayer.MyErrorListener() {
            @Override
            public void onError(int errorCode) {
                switch (errorCode) {
                    case 0:
                        break;
                }
            }
        });

    }

    @Override
    protected void onResume() {
        super.onResume();
        player.prepare();
    }
}
