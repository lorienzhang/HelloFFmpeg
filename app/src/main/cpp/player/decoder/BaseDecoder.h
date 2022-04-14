#ifndef HELLOFFMPEG_BASEDECODER_H
#define HELLOFFMPEG_BASEDECODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavcodec/jni.h>
}

#include <thread>
#include <jni.h>

#include <LogUtil.h>
#include "Decoder.h"

#define MAX_PATH   2048
#define DELAY_THRESHOLD 100 //100ms

using namespace std;

enum DecoderState {
    STATE_UNKNOWN,
    STATE_DECODING,
    STATE_PAUSE,
    STATE_STOP
};

enum DecoderMsg {
    MSG_DECODER_INIT_ERROR,
    MSG_DECODER_READY,
    MSG_DECODER_DONE,
    MSG_DECODER_RENDER,
    MSG_DECODING_TIME
};

class BaseDecoder : public Decoder {
public:
    BaseDecoder() {};
    virtual ~BaseDecoder() {};

    virtual void start();
    virtual void pause();
    virtual void stop();
    virtual float getDuration() {
        // ms to s
        return mDuration * 1.0f / 1000;
    }
    virtual void seekToPosition(float position);
    virtual float getCurrentPosition();
    virtual void clearCache() {};
    virtual void setMessageCallback(void *context, MessageCallback callback) {
        mMsgContext = context;
        mMsgCallback = callback;
    }

protected:
    // 消息上下文指针，外部设置，一般是Player指针
    void *mMsgContext = nullptr;
    // 消息处理函数指针，外部设置
    MessageCallback mMsgCallback = nullptr;
    virtual int init(const char *url, AVMediaType type);
    virtual void unInit();
    virtual void onDecoderReady() = 0;
    virtual void onDecoderDone() = 0;
    virtual void onFrameAvailable(AVFrame *frame) = 0;

    AVCodecContext *getCodecContext() {
        return mAVCodecContext;
    }

private:
    int initFFDecoder();

    void unInitDecoder();
    // 启动解码线程
    void startDecodingThread();
    // 音视频循环解码
    void decodingLoop();
    // 解码一个patcket
    int decodeOnePacket();
    // 更新当前mCurTimeStamp
    void updateTimeStamp();
    // 音视频同步
    long AVSync();
    // 线程执行函数
    static void doAVDecode(BaseDecoder *decoder);

    // 封装格式上下文
    AVFormatContext *mAVFormatContext = nullptr;
    // 解码器上下文
    AVCodecContext *mAVCodecContext = nullptr;
    // 流类型
    AVMediaType mMediaType = AVMEDIA_TYPE_UNKNOWN;
    // 解码器
    const AVCodec *mAVCodec = nullptr;
    // 编码数据包
    AVPacket *mAVPacket = nullptr;
    // 解码后的帧
    AVFrame *mFrame = nullptr;

    // 记录流的索引
    int mStreamIndex = -1;
    // 总时长 ms，在initFFDecoder()中设置它
    long mDuration = 0;
    // 文件地址
    char mUrl[MAX_PATH] = {0};
    // 当前播放时间
    long mCurTimeStamp = 0;
    // 起播时间
    long mStartTimeStamp = -1;
    // 线程指针
    thread *mThread = nullptr;
    // 锁
    mutex mMutex;
    // 条件变量
    condition_variable mCond;
    // 解码器状态
    volatile int mDecoderState = STATE_UNKNOWN;
    // seek position
    volatile float mSeekPosition = 0;
    volatile bool mSeekSuccess = false;
};

#endif //HELLOFFMPEG_BASEDECODER_H
