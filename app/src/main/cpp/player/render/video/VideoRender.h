#ifndef HELLOFFMPEG_VIDEORENDER_H
#define HELLOFFMPEG_VIDEORENDER_H

#define VIDEO_RENDER_ANWINDOW       0
#define VIDEO_RENDER_OPENGL         1

#include "ImageDef.h"

class VideoRender {
public:
    VideoRender(int type) {
        mRenderType = type;
    }
    virtual ~VideoRender() {}
    virtual void init(int videoWidth, int videoHeight, int *dstSize) = 0;
    virtual void unInit() = 0;
    virtual void renderVideoFrame(NativeImage *pImage) = 0;

    int getRenderType() {
        return mRenderType;
    }
private:
    int mRenderType = VIDEO_RENDER_ANWINDOW;
};

#endif //HELLOFFMPEG_VIDEORENDER_H
