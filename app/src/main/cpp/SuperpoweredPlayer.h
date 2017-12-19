//
// Created by Jaeyoung Park on 19/12/2017.
//

#ifndef PLAYER_SUPERPOWEREDPLAYER_H
#define PLAYER_SUPERPOWEREDPLAYER_H
#include <jni.h>
#include <android/log.h>
#include <AndroidIO/SuperpoweredAndroidAudioIO.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <malloc.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

extern "C" {
JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_open(
        JNIEnv *env, jobject instance, jstring pathOfFile);
JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onPlayPause(
        JNIEnv *env, jobject instance, jboolean play);
JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onSeek(
        JNIEnv *env, jobject instance, jfloat percent);
JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onFilterOnoff(
        JNIEnv *env, jobject instance, jboolean onoff);
}

static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value);
static bool audioProcessing(void __unused *clientdata, short int *audioIO,
                            int numberOfSamples, int __unused samplerate);

class SuperpoweredPlayer {
public:
    SuperpoweredPlayer(unsigned int samplerate, unsigned int buffersize);

    ~SuperpoweredPlayer();

    void onPlayPause(bool play);
    void onSeek(float percent);

    bool process(short *audioIO, unsigned int numberOfSamples);
    void open(char *string);
    void eventCallback(SuperpoweredAdvancedAudioPlayerEvent event, void *value);

private:
    SuperpoweredAndroidAudioIO *audioSystem;
    SuperpoweredAdvancedAudioPlayer *player;
    float *stereoBuffer;
    float vol = 1.0f;
    unsigned int samplerate, buffersize;
    char *path;
};

#endif //PLAYER_SUPERPOWEREDPLAYER_H
