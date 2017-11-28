#include "native-lib.h"
#include <jni.h>
#include <string>
#include <stdlib.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
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
    return ((AudioPlayerImpl *)clientdata)->process(audioIO, (unsigned int)numberOfSamples);
}

static void playerEventCallback(void *clientData, SuperpoweredAdvancedAudioPlayerEvent event, void * __unused value) {
    if (event == SuperpoweredAdvancedAudioPlayerEvent_LoadSuccess) {
        SuperpoweredAdvancedAudioPlayer *player = *((SuperpoweredAdvancedAudioPlayer **)clientData);
        player->setBpm(126.0f);
        player->setFirstBeatMs(353);
        player->setPosition(player->firstBeatMs, false, false);
    };
}

bool AudioPlayerImpl::process(short int *output, unsigned int numberOfSamples) {
    bool silence = !player->process(stereoBuffer, false, numberOfSamples, vol);

    if (!silence) SuperpoweredFloatToShortInt(stereoBuffer, output, numberOfSamples);
    return !silence;
}

extern "C"
JNIEXPORT void
Java_com_asuscomm_yangyinetwork_player_MainActivity_audioInitialize(JNIEnv *javaEnvironment, jobject __unused obj, jint samplerate, jint buffersize, jstring apkPath, jint fileOffset, jint fileLength) {
    const char *path = javaEnvironment->GetStringUTFChars(apkPath, JNI_FALSE);
    example = new AudioPlayerImpl((unsigned int)samplerate, (unsigned int)buffersize, path, fileOffset, fileLength);
    javaEnvironment->ReleaseStringUTFChars(apkPath, path);
}

void AudioPlayerImpl::onPlayPause(bool play) {

    if (!play) {
        player->pause();
    } else {
        player->play(false);
    };
    SuperpoweredCPU::setSustainedPerformanceMode(play); // <-- Important to prevent audio dropouts.
}

AudioPlayerImpl::AudioPlayerImpl(unsigned int samplerate, unsigned int buffersize, const char *path, int fileOffset, int fileLength) {
    stereoBuffer = (float *)memalign(16, (buffersize + 16) * sizeof(float) * 2);

    player = new SuperpoweredAdvancedAudioPlayer(&player , playerEventCallback, samplerate, 0);

    player->open(path, fileOffset, fileLength);

    audioSystem = new SuperpoweredAndroidAudioIO(samplerate, buffersize, false, true, audioProcessing, this, -1, SL_ANDROID_STREAM_MEDIA, buffersize * 2);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_asuscomm_yangyinetwork_player_MainActivity_onPlayPause(JNIEnv *env, jobject instance,
                                                                jboolean play) {
    example->onPlayPause(play);



}