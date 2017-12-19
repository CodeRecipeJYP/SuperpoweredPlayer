//
// Created by Jaeyoung Park on 19/12/2017.
//


#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include "SuperpoweredPlayer.h"

static const char* TAG = "SuperpoweredPlayer.cpp";

SuperpoweredPlayer::SuperpoweredPlayer(unsigned int samplerate, unsigned int buffersize) {
    this->samplerate = samplerate;
    this->buffersize = buffersize;
}

SuperpoweredPlayer::~SuperpoweredPlayer() {
    delete audioSystem;
    delete player;
    free(stereoBuffer);
}

void SuperpoweredPlayer::open(char *path) {
    this->path = path;
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);
    player = new SuperpoweredAdvancedAudioPlayer(&(player), playerEventCallback, samplerate, 0);
    player->open(this->path);

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this,
                                                 -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
}

static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value) {
    SuperpoweredPlayer *playerInstance = (SuperpoweredPlayer *) clientData;
    playerInstance->eventCallback(event, value);
}

void SuperpoweredPlayer::eventCallback(SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG,
                            "playerEventCallback: SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess");
//        SuperpoweredAdvancedAudioPlayer *player = *((SuperpoweredAdvancedAudioPlayer **)clientData);
//        player->setPosition(player->firstBeatMs, false, false);
    };
}

static bool audioProcessing(void __unused *clientdata, short *audioIO,
                            int numberOfSamples, int __unused samplerate) {
    SuperpoweredPlayer *playerInstance = (SuperpoweredPlayer *) clientdata;
    return (playerInstance->process(audioIO, numberOfSamples));
}

bool SuperpoweredPlayer::process(short *audioIO, unsigned int numberOfSamples) {
    bool silence = !player->process(stereoBuffer, false, numberOfSamples, vol);

    if (!silence) SuperpoweredFloatToShortInt(stereoBuffer, audioIO, numberOfSamples);

    return !silence;
}

void SuperpoweredPlayer::onPlayPause(bool play) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "AudioPlayerImpl::onSeek play=%s", play ? "True" : "False");
    if (!play) {
        player->pause();
    } else {
        player->play(false);
    };
    SuperpoweredCPU::setSustainedPerformanceMode(play); // <-- Important to prevent audio dropouts.
}

void SuperpoweredPlayer::onSeek(float percent) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "AudioPlayerImpl::onSeek percent=%f", percent);
    player->seek(percent);
}

static SuperpoweredPlayer *playerInstance = NULL;
JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_initialize(
        JNIEnv *env, jobject instance, jint samplerate, jint buffersize) {

    playerInstance = new SuperpoweredPlayer((unsigned int) samplerate, (unsigned int) buffersize);
}

JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_open(
        JNIEnv *env, jobject instance, jstring pathOfFile) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG,
                        "Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_open");

    char *path = (char*) env->GetStringUTFChars(pathOfFile, JNI_FALSE);
    playerInstance->open(path);
}

JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onPlayPause(
        JNIEnv *env, jobject instance, jboolean play) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG,
                        "Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onPlayPause");
    playerInstance->onPlayPause(play);
}
JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onSeek(
        JNIEnv *env, jobject instance, jfloat percent) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG,
                        "Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onSeek");
    playerInstance->onSeek(percent);
}

JNIEXPORT void JNICALL Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onFilterOnoff(
        JNIEnv *env, jobject instance, jboolean onoff) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG,
                        "Java_com_asuscomm_yangyinetwork_player_jni_SuperpoweredPlayer_onFilterOnoff");

}