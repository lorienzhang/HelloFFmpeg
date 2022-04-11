#ifndef HELLOFFMPEG_FFMEDIAPLAYER_H
#define HELLOFFMPEG_FFMEDIAPLAYER_H

#include "MediaPlayer.h"
#include "decoder/VideoDecoder.h"
#include "render/video/VideoRender.h"

class FFMediaPlayer : public MediaPlayer {
public:
    FFMediaPlayer() {};
    virtual ~FFMediaPlayer() {};

    virtual void init(JNIEnv *env, jobject obj, char *url, int renderType, jobject surface);

    virtual void unInit();

    virtual void play();
    virtual void pause();
    virtual void stop();
    virtual void seekToPosition(float position);
    virtual long getMediaParams(int paramType);

private:

    virtual jobject getJavaObj();
    virtual JavaVM *getJavaVM();
    virtual JNIEnv *getJNIEnv(bool *isAttach);

    // 向java发送消息
    static void postMessage(void *context, int msgType, float msgCode);

    VideoDecoder *mVideoDecoder = nullptr;
    VideoRender *mVideoRender = nullptr;
};
#endif //HELLOFFMPEG_FFMEDIAPLAYER_H
