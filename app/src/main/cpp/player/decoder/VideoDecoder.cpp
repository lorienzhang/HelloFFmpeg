#include "VideoDecoder.h"
#include "../../util/LogUtil.h"

void VideoDecoder::onDecoderReady() {
    LOGD("VideoDecoder::onDecoderReady");
    mVideoWidth = getCodecContext()->width;
    mVideoHeight = getCodecContext()->height;

    if (mMsgContext && mMsgCallback) {
        mMsgCallback(mMsgContext, MSG_DECODER_READY, 0);
    }

    // todo: Render
}

void VideoDecoder::onDecoderDone() {
    LOGD("VideoDecoder::onDecoderDone");
    if (mMsgContext && mMsgCallback) {
        mMsgCallback(mMsgContext, MSG_DECODER_DONE, 0);
    }
}

void VideoDecoder::onFrameAvailable(AVFrame *frame) {
    LOGD("VideoDecoder::onFrameAvailable frame=%p", frame);
}