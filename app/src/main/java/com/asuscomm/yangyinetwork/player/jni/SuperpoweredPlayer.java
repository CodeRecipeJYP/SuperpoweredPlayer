package com.asuscomm.yangyinetwork.player.jni;

/**
 * Created by jaeyoung on 19/12/2017.
 */

public class SuperpoweredPlayer {
    private native void open(String pathOfFile);
    private native void onPlayPause(boolean play);
    private native void onSeek(float percent);
    private native void onFilterOnoff(boolean onoff);
}
