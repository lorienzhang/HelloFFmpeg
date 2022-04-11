#include "VideoDecoder.h"

/**
 * decoder初始化完毕调用该方法，这里还没开始解析
 */
void VideoDecoder::onDecoderReady() {
    LOGD("VideoDecoder::onDecoderReady");
    mVideoWidth = getCodecContext()->width;
    mVideoHeight = getCodecContext()->height;

    if (mMsgContext && mMsgCallback) {
        mMsgCallback(mMsgContext, MSG_DECODER_READY, 0);
    }

    // init VideoRender
    if (mVideoRender != nullptr) {
        int dstSize[2] = {0};
        mVideoRender->init(mVideoWidth, mVideoHeight, dstSize);
        mRenderWidth = dstSize[0];
        mRenderHeight = dstSize[1];

        mRGBAFrame = av_frame_alloc();
        int bufferSize = av_image_get_buffer_size(DST_PIXEL_FORMAT, mRenderWidth, mRenderHeight, 1);
        mFrameBuffer = (uint8_t *) av_malloc(bufferSize * sizeof(uint8_t));
        av_image_fill_arrays(mRGBAFrame->data, mRGBAFrame->linesize,
                             mFrameBuffer, DST_PIXEL_FORMAT,
                             mRenderWidth, mRenderHeight, 1);
        LOGD("VideoDecoder::onDecoderReady sws_getContext, [srcW,srcH]=[%d, %d], [dstW,dstH]=[%d, %d]",
             mVideoWidth, mVideoHeight, mRenderWidth, mRenderHeight);
        // scale之后，YUV数据转成了RGB，这样就可将图像数据刷到ANativeWindow上
        mSwsContext = sws_getContext(mVideoWidth, mVideoHeight, getCodecContext()->pix_fmt,
                                     mRenderWidth, mRenderHeight, DST_PIXEL_FORMAT,
                                     SWS_FAST_BILINEAR, NULL, NULL, NULL);
    } else {
        LOGE("VideoDecoder::onDecoderReady mVideoRender = null");
    }
}

void VideoDecoder::onFrameAvailable(AVFrame *frame) {
    LOGD("VideoDecoder::onFrameAvailable frame=%p", frame);
    if (mVideoRender != nullptr && frame != nullptr) {
        LOGD("VideoDecoder::onFrameAvailable frame[w,h]=[%d, %d], fmt=%d, [line2,line1,line2]=[%d, %d, %d]\n",
             frame->width, frame->height, getCodecContext()->pix_fmt,
             frame->linesize[0], frame->linesize[1], frame->linesize[2]);
        NativeImage image;
        if (mVideoRender->getRenderType() == VIDEO_RENDER_ANWINDOW) {
            sws_scale(mSwsContext, frame->data, frame->linesize, 0, mVideoHeight,
                      mRGBAFrame->data, mRGBAFrame->linesize);
            image.format = IMAGE_FORMAT_RGBA;
            image.width = mRenderWidth;
            image.height = mRenderHeight;
            image.ppPlane[0] = mRGBAFrame->data[0];
            image.pLineSize[0] = image.width * 4;
        }

        mVideoRender->renderVideoFrame(&image);
    }

    if (mMsgContext && mMsgCallback) {
        mMsgCallback(mMsgContext, MSG_DECODER_RENDER, 0);
    }
}

void VideoDecoder::onDecoderDone() {
    LOGD("VideoDecoder::onDecoderDone");
    if (mMsgContext && mMsgCallback) {
        mMsgCallback(mMsgContext, MSG_DECODER_DONE, 0);
    }

    if (mVideoRender) {
        mVideoRender->unInit();
    }

    if (mRGBAFrame != nullptr) {
        av_frame_free(&mRGBAFrame);
        mRGBAFrame = nullptr;
    }

    if (mFrameBuffer != nullptr) {
        free(mFrameBuffer);
        mFrameBuffer = nullptr;
    }

    if (mSwsContext != nullptr) {
        sws_freeContext(mSwsContext);
        mSwsContext = nullptr;
    }
}