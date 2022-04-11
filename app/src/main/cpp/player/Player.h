#ifndef HELLOFFMPEG_PLAYER_H
#define HELLOFFMPEG_PLAYER_H
#include <jni.h>
#include "MediaPlayer.h"

class Player {
public:
    Player() {};
    virtual ~Player() {};

    void init(JNIEnv *jniEnv, jobject obj, char *url, int renderType, jobject surface);
    void unInit();
    void play();
    void pause();
    void stop();
    void seekToPosition(float position);
    long getMediaParams(int paramType);

private:
    MediaPlayer *mMediaPlayer = nullptr;
};

#endif //HELLOFFMPEG_PLAYER_H
