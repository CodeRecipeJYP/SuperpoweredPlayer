#include "native-lib.h"
#include <jni.h>
#include <string>
#include <stdlib.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <android/log.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES.h>


static AudioPlayerImpl *example = NULL;

extern "C"
JNIEXPORT jstring
JNICALL
Java_com_asuscomm_yangyinetwork_player_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

static bool audioProcessing(void *clientdata, short int *audioIO, int numberOfSamples, int __unused samplerate) {
//    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "audioProcessing");
    return ((AudioPlayerImpl *)clientdata)->process(audioIO, (unsigned int)numberOfSamples);
}

static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "playerEventCallback");
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        SuperpoweredAdvancedAudioPlayer *player = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        player->setBpm(126.0f);
        player->setFirstBeatMs(353);
        player->setPosition(player->firstBeatMs, false, false);
    };
}

bool AudioPlayerImpl::process(short int *output, unsigned int numberOfSamples) {
//    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::process");

    // Issue02, volume도 0이었음
    vol = 1.0f;

    // Issue01, silence 를 false로 해서 문제생긴거였음
    // this->player->process(stereoBuffer, false, numberOfSamples, vol);
    // -> this->player->process(stereoBuffer, tmp_silence, numberOfSamples, vol);
    bool tmp_silence = true;
    bool silence = !this->player->process(stereoBuffer, tmp_silence, numberOfSamples, vol);

    __android_log_print(ANDROID_LOG_DEBUG, "SuperpoweredExample.cpp", "SuperpoweredExample::process Silence=%d, Vol=%lf", false, vol);
    if (!silence) SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    __android_log_print(ANDROID_LOG_DEBUG, "SuperpoweredExample.cpp", "SuperpoweredExample::process Return !Silence=%d", !silence);
    return !silence;
}

extern "C"
JNIEXPORT void
Java_com_asuscomm_yangyinetwork_player_MainActivity_audioInitialize(JNIEnv *javaEnvironment, jobject __unused obj, jint samplerate, jint buffersize, jstring apkPath, jint fileOffset, jint fileLength) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "Java_com_asuscomm_yangyinetwork_player_MainActivity_audioInitialize");
    const char *path = javaEnvironment->GetStringUTFChars(apkPath, JNI_FALSE);
    example = new AudioPlayerImpl((unsigned int)samplerate, (unsigned int)buffersize, path, fileOffset, fileLength);
    javaEnvironment->ReleaseStringUTFChars(apkPath, path);
}

void AudioPlayerImpl::onPlayPause(bool play) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::onPlayPause");

    if (!play) {
        this->player->pause();
    } else {
        this->player->play(false);
    };
    SuperpoweredCPU::setSustainedPerformanceMode(play); // <-- Important to prevent audio dropouts.
}

AudioPlayerImpl::AudioPlayerImpl(unsigned int samplerate, unsigned int buffersize, const char *path, int fileOffset, int fileLength) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::AudioPlayerImpl");
    this->stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    this->player = new SuperpoweredAdvancedAudioPlayer(&(this->player), playerEventCallback, samplerate, 0);

    this->player->open(path, fileOffset, fileLength);
    this->player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;

    this->audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_asuscomm_yangyinetwork_player_MainActivity_onPlayPause(JNIEnv *env, jobject instance,
                                                                jboolean play) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "Java_com_asuscomm_yangyinetwork_player_MainActivity_onPlayPause");
    example->onPlayPause(play);



}