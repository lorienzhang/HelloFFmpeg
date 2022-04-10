#ifndef HELLOFFMPEG_VIDEODECODER_H
#define HELLOFFMPEG_VIDEODECODER_H

#include "BaseDecoder.h"

class VideoDecoder : public BaseDecoder {

public:
    VideoDecoder(char *url) {
        init(url, AVMEDIA_TYPE_VIDEO);
    }

    virtual ~VideoDecoder() {
        unInit();
    }

private:
    virtual void onDecoderReady();
    virtual void onDecoderDone();
    virtual void onFrameAvailable(AVFrame *frame);

    int mVideoWidth = 0;
    int mVideoHeight = 0;
};

#endif //HELLOFFMPEG_VIDEODECODER_H
