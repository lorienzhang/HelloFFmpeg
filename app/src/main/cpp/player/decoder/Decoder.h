#ifndef HELLOFFMPEG_DECODER_H
#define HELLOFFMPEG_DECODER_H

/** 消息回调，将播放器关键状态，通知java层 */
typedef void (*MessageCallback)(void *, int, float);

class Decoder {
public:
    virtual void start() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual float getDuration() = 0;
    virtual void seekToPosition(float position) = 0;
    virtual float getCurrentPosition() = 0;
    virtual void setMessageCallback(void *context, MessageCallback callback) = 0;
};
#endif //HELLOFFMPEG_DECODER_H
