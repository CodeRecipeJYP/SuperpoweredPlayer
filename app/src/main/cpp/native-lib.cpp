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
#include <math.h>


#define HEADROOM_DECIBEL 3.0f
static const float headroom = powf(10.0f, -HEADROOM_DECIBEL * 0.025f);
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
        __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "playerEventCallback SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess");
        SuperpoweredAdvancedAudioPlayer *player = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        player->setBpm(126.0f);
        player->setFirstBeatMs(353);
        player->setPosition(player->firstBeatMs, false, false);
    };
}

bool AudioPlayerImpl::process(short int *output, unsigned int numberOfSamples) {
//    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::process");
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::process numberOfSamples=%d", numberOfSamples);

    // Issue02, volume도 0이었음
    vol = 1.0f * headroom;

    // Issue03
//    double masterBpm = player->currentBpm;
//    double masterBpm = 0.0f;
//    double msElapsedSinceLastBeat = player->msElapsedSinceLastBeat;

    // Issue01, silence 를 false로 해서 문제생긴거였음
    // player->process(stereoBuffer, false, numberOfSamples, vol);
    // -> player->process(stereoBuffer, tmp_silence, numberOfSamples, vol);
    bool tmp_silence = true;
//    bool silence = !player->process(stereoBuffer, tmp_silence, numberOfSamples, vol, masterBpm, msElapsedSinceLastBeat);
//    bool silence = !player->process(stereoBuffer, tmp_silence, numberOfSamples, vol);
    bool silence = !player->process(stereoBuffer, false, numberOfSamples, vol);

//    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "SuperpoweredExample::process Silence=%d, Vol=%lf, MasterBpm=%lf, MsElapsedSinceLastBeat=%lf", false, vol, masterBpm, msElapsedSinceLastBeat);
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "SuperpoweredExample::process Silence=%d, Vol=%lf", false, vol);
    if (!silence) SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "SuperpoweredExample::process Return !Silence=%d", !silence);
    return !silence;
}

extern "C"
JNIEXPORT void
Java_com_asuscomm_yangyinetwork_player_MainActivity_audioInitialize(JNIEnv *javaEnvironment, jobject __unused obj, jint samplerate, jint buffersize, jstring apkPath, jint fileOffset, jint fileLength) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "Java_com_asuscomm_yangyinetwork_player_MainActivity_audioInitialize");
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "Java_com_asuscomm_yangyinetwork_player_MainActivity_audioInitialize/ samplerate=%d, buffersize=%d", samplerate, buffersize);
    const char *path = javaEnvironment->GetStringUTFChars(apkPath, JNI_FALSE);
    example = new AudioPlayerImpl((unsigned int)samplerate, (unsigned int)buffersize, path, fileOffset, fileLength);
    javaEnvironment->ReleaseStringUTFChars(apkPath, path);
}


// issue11 path
extern "C"
JNIEXPORT void
Java_com_asuscomm_yangyinetwork_player_MainActivity_audioInitializeWithPath(JNIEnv *javaEnvironment, jobject __unused obj, jint samplerate, jint buffersize, jstring filePath) {
    const char *path = javaEnvironment->GetStringUTFChars(filePath, JNI_FALSE);
    example = new AudioPlayerImpl((unsigned int) samplerate, (unsigned int)buffersize, path);
    javaEnvironment->ReleaseStringUTFChars(filePath, path);
}

void AudioPlayerImpl::onPlayPause(bool play) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::onPlayPause");

    if (!play) {
        player->pause();
    } else {
// issue4 play(false)기존 -> true
//        player->play(true);
        player->play(false);
    };
    SuperpoweredCPU::setSustainedPerformanceMode(play); // <-- Important to prevent audio dropouts.
}

AudioPlayerImpl::AudioPlayerImpl(unsigned int samplerate, unsigned int buffersize, const char *path, int fileOffset, int fileLength) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::AudioPlayerImpl");
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    player = new SuperpoweredAdvancedAudioPlayer(&(player), playerEventCallback, samplerate, 0);

    player->open(path, fileOffset, fileLength);
// issue5 sync SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat(기존) -> None
//    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_TempoAndBeat;
//    player->syncMode = SuperpoweredAdvancedAudioPlayerSyncMode_None;

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
}

AudioPlayerImpl::AudioPlayerImpl(unsigned int samplerate, unsigned int buffersize,
                                 const char *path) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "AudioPlayerImpl::AudioPlayerImpl(unsigned int samplerate, unsigned int buffersize, const char *path)");
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);
    player = new SuperpoweredAdvancedAudioPlayer(&(player), playerEventCallback, samplerate, 0);
    player->open(path);
    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_asuscomm_yangyinetwork_player_MainActivity_onPlayPause(JNIEnv *env, jobject instance,
                                                                jboolean play) {
    __android_log_print(ANDROID_LOG_DEBUG, "native-lib.cpp", "Java_com_asuscomm_yangyinetwork_player_MainActivity_onPlayPause");
    example->onPlayPause(play);



}