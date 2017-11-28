package com.asuscomm.yangyinetwork.player;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.AudioManager;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        initPlayer();


    }

    private void initPlayer() {

        //region initAssets
        String samplerateString = null, buffersizeString = null;
        if (Build.VERSION.SDK_INT >= 17) {
            AudioManager audioManager = (AudioManager) this.getSystemService(Context.AUDIO_SERVICE);
            samplerateString = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            buffersizeString = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        }
        if (samplerateString == null) samplerateString = "44100";
        if (buffersizeString == null) buffersizeString = "512";

        // Files under res/raw are not zipped, just copied into the APK. Get the offset and length to know where our files are located.
        AssetFileDescriptor fd0 = getResources().openRawResourceFd(R.raw.lycka);
        int fileOffset = (int)fd0.getStartOffset(), fileLength = (int)fd0.getLength();
        try {
            fd0.getParcelFileDescriptor().close();
        } catch (IOException e) {
            android.util.Log.d("", "Close error.");
        }
        //endregion

        audioStart(Integer.parseInt(samplerateString),Integer.parseInt(buffersizeString),getPackageResourcePath(),fileOffset,fileLength);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void audioStart(int samplerate, int buffersize, String apkPath, int fileOffset, int fileLength);
}
