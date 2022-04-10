
#ifndef HELLOFFMPEG_MEDIAPLAYER_H
#define HELLOFFMPEG_MEDIAPLAYER_H

#include <jni.h>

#define JAVA_PLAYER_EVENT_CALLBACK_API_NAME "playerEventCallback"

class MediaPlayer {
public:
    MediaPlayer() {};
    virtual ~MediaPlayer(){};

    virtual void init(JNIEnv *env, jobject obj, char *url, int renderType, jobject surface) = 0;

    virtual void unInit() = 0;

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seekToPosition(float position) = 0;

    virtual JNIEnv *getJNIEnv(bool *isAttach) = 0;
    virtual jobject getJavaObj() = 0;
    virtual JavaVM *getJavaVM() = 0;

    JavaVM *mJavaVM = nullptr;
    // 存储java obj
    jobject mJavaObj = nullptr;
};

#endif //HELLOFFMPEG_MEDIAPLAYER_H
