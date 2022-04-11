#ifndef HELLOFFMPEG_VIDEODECODER_H
#define HELLOFFMPEG_VIDEODECODER_H

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/jni.h>
}

#include "BaseDecoder.h"
#include "../render/video/VideoRender.h"
#include "../../util/LogUtil.h"

class VideoDecoder : public BaseDecoder {

public:
    VideoDecoder(char *url) {
        init(url, AVMEDIA_TYPE_VIDEO);
    }

    virtual ~VideoDecoder() {
        unInit();
    }

    void setVideoRender(VideoRender *videoRender) {
        mVideoRender = videoRender;
    }

    int getVideoWidth() {
        return mVideoWidth;
    }

    int getVideoHeight() {
        return mVideoHeight;
    }

private:
    virtual void onDecoderReady();
    virtual void onDecoderDone();
    virtual void onFrameAvailable(AVFrame *frame);

    const AVPixelFormat DST_PIXEL_FORMAT = AV_PIX_FMT_RGBA;

    int mVideoWidth = 0;
    int mVideoHeight = 0;

    int mRenderWidth = 0;
    int mRenderHeight = 0;

    AVFrame *mRGBAFrame = nullptr;
    uint8_t *mFrameBuffer = nullptr;

    // 关联VideoRender
    VideoRender *mVideoRender = nullptr;
    SwsContext *mSwsContext = nullptr;
};

#endif //HELLOFFMPEG_VIDEODECODER_H
