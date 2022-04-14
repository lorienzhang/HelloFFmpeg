#include "AudioDecoder.h"

/**
 * 设置音频解码相关参数
 */
void AudioDecoder::onDecoderReady() {
    LOGD("AudioDecoder::onDecoderReady");
    if (mAudioRender) {
        AVCodecContext *codecContext = getCodecContext();
        mSwrContext = swr_alloc();

        av_opt_set_int(mSwrContext, "in_channel_layout", codecContext->channel_layout, 0);
        // AV_CH_LAYOUT_STEREO
        av_opt_set_int(mSwrContext, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);

        av_opt_set_int(mSwrContext, "in_sample_rate", codecContext->sample_rate, 0);
        // 44100
        av_opt_set_int(mSwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

        av_opt_set_int(mSwrContext, "in_sample_fmt", codecContext->sample_fmt, 0);
        // AV_SAMPLE_FMT_S16
        av_opt_set_int(mSwrContext, "out_sample_fmt", DST_SAMPLE_FORMAT, 0);

        swr_init(mSwrContext);

        LOGD("AudioDecoder::onDecoderReady audio metadata, sampleRate: %d, channel: %d, format: %d, frame_size: %d, layout: %lld",
             codecContext->sample_rate, codecContext->channels, codecContext->sample_fmt,
             codecContext->frame_size, codecContext->channel_layout);

        // resample
        mNbSample = av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE,
                                   codecContext->sample_rate,AV_ROUND_UP);
        mDstFrameDataSize = av_samples_get_buffer_size(NULL, AUDIO_DST_CHANNEL_COUNTS,
                                                       mNbSample, DST_SAMPLE_FORMAT, 1);
        // 分配OpenSL播放音频的帧内存
        mAudioOutBuffer = (uint8_t *) malloc(mDstFrameDataSize);

        // 音频渲染初始化，内部会起个线程专门渲染音频
        mAudioRender->init();
    } else {
        LOGD("AudioDecoder::onDecoderReady mAudioRender == null");
    }
}

void AudioDecoder::onDecoderDone() {
    LOGD("AudioDecoder::onDecoderDone");
    if (mAudioRender) {
        mAudioRender->unInit();
    }

    if (mAudioOutBuffer) {
        free(mAudioOutBuffer);
        mAudioOutBuffer = nullptr;
    }

    if (mSwrContext) {
        swr_free(&mSwrContext);
        mSwrContext = nullptr;
    }
}

void AudioDecoder::onFrameAvailable(AVFrame *frame) {
    LOGD("AudioDecoder::onFrameAvailable frame=%p, frame->nb_samples=%d\n", frame, frame->nb_samples);
    if (mAudioRender) {
        // 将解码出来音频帧进行重采样，最终数据存入mAudioOutBuffer中
        int result = swr_convert(mSwrContext, &mAudioOutBuffer,mDstFrameDataSize / 2,
                                 (const uint8_t **)frame->data, frame->nb_samples);
        if (result > 0) {
            mAudioRender->renderAudioFrame(mAudioOutBuffer, mDstFrameDataSize);
        }
    }
}

void AudioDecoder::clearCache() {
    if (mAudioRender) {
        mAudioRender->clearAudioCache();
    }
}
