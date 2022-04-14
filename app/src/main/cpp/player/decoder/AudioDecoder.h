#ifndef HELLOFFMPEG_AUDIODECODER_H
#define HELLOFFMPEG_AUDIODECODER_H

extern "C" {
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
}

#include "Decoder.h"
#include "BaseDecoder.h"
#include "../render/audio/AudioRender.h"
#include "../render/audio/OpenSLRender.h"

/** 音频编码采样率 */
static const int AUDIO_DST_SAMPLE_RATE = 44100;
/** 音频编码通道数 */
static const int AUDIO_DST_CHANNEL_COUNTS = 2;
/** 音频编码声道格式 */
static const uint64_t AUDIO_DST_CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
/** ACC音频一帧采样数 */
static const int ACC_NB_SAMPLES = 1024;

class AudioDecoder : public BaseDecoder {
public:

    AudioDecoder(char *url) {
        init(url, AVMEDIA_TYPE_AUDIO);
    }

    virtual ~AudioDecoder() {
        unInit();
    }

    void setAudioRender(AudioRender *audioRender) {
        mAudioRender = audioRender;
    }

private:

    virtual void onDecoderReady();
    virtual void onDecoderDone();
    virtual void onFrameAvailable(AVFrame *frame);
    virtual void clearCache();

    // 采样格式，16位
    const AVSampleFormat DST_SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;

    AudioRender *mAudioRender = nullptr;

    // 音频重采样上下文
    SwrContext *mSwrContext = nullptr;
    // 音频数据内容
    uint8_t *mAudioOutBuffer = nullptr;

    int mNbSample = 0;
    int mDstFrameDataSize = 0;
};

#endif //HELLOFFMPEG_AUDIODECODER_H
