//
// Created by Jaeyoung Park on 28/11/2017.
//

#ifndef PLAYER_NATIVE_LIB_H
#define PLAYER_NATIVE_LIB_H

#include <AndroidIO/SuperpoweredAndroidAudioIO.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include <SuperpoweredFilter.h>


class AudioPlayerImpl {
public:

    AudioPlayerImpl(unsigned int samplerate, unsigned int buffersize, const char *path, int fileOffset, int fileLength);

    AudioPlayerImpl(unsigned int samplerate, unsigned int buffersize, const char *path);

    ~AudioPlayerImpl();

    bool process(short int *output, unsigned int numberOfSamples);
    void onPlayPause(bool play);
    void onCrossfader(int value);
    void onFxSelect(int value);
    void onFxOff();
    void onFxValue(int value);
    void onEQBand(unsigned int index, int gain);
    SuperpoweredFilter *filter;

private:
    SuperpoweredAndroidAudioIO *audioSystem;
    SuperpoweredAdvancedAudioPlayer *player;
    float *stereoBuffer;
    float vol;
};

#endif //PLAYER_NATIVE_LIB_H
