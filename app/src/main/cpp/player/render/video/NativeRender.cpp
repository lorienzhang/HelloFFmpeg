#include <string.h>

#include "NativeRender.h"
#include "LogUtil.h"

NativeRender::NativeRender(JNIEnv *env, jobject surface) : VideoRender(VIDEO_RENDER_ANWINDOW) {
    mNativeWindow = ANativeWindow_fromSurface(env, surface);
}

NativeRender::~NativeRender() {
    if (mNativeWindow) {
        ANativeWindow_release(mNativeWindow);
    }
}

void NativeRender::init(int videoWidth, int videoHeight, int *dstSize) {
    LOGD("NativeRender::init, video[w, h]=[%d, %d]\n", videoWidth, videoHeight);
    if (mNativeWindow == nullptr) {
        return;
    }

    int windowWidth = ANativeWindow_getWidth(mNativeWindow);
    int windowHeight = ANativeWindow_getHeight(mNativeWindow);

    // 处理视频过宽or过高，导致window展示不下
    if (windowWidth < windowHeight * videoWidth / videoHeight) {
        // 视频过宽
        mDstWidth = windowWidth;
        mDstHeight = windowWidth * videoHeight / videoWidth;
    } else {
        mDstHeight = windowHeight;
        mDstWidth = windowHeight * videoWidth / videoHeight;
    }
    LOGD("NativeRender::init, window[w,h=[%d, %d], dstSize[w, h]=[%d, %d]\n",windowWidth, windowHeight, mDstWidth, mDstHeight);

    ANativeWindow_setBuffersGeometry(mNativeWindow, mDstWidth, mDstHeight, WINDOW_FORMAT_RGBA_8888);

    dstSize[0] = mDstWidth;
    dstSize[1] = mDstHeight;
}

void NativeRender::unInit() {

}

/**
 * 将视频一帧数据填充到framebuffer中
 *
 * @param pImage
 */
void NativeRender::renderVideoFrame(NativeImage *pImage) {
    if (mNativeWindow == nullptr || pImage == nullptr) {
        return;
    }

    ANativeWindow_lock(mNativeWindow, &mNativeWindowBuffer, nullptr);
    uint8_t *dstBuffer = static_cast<uint8_t *>(mNativeWindowBuffer.bits);

    int srcLineSize = pImage->width * 4;
    int dstLineSize = mNativeWindowBuffer.stride * 4;

    for (int i = 0; i < mDstHeight; ++i) {
        memcpy(dstBuffer + i * dstLineSize, pImage->ppPlane[0] + i * srcLineSize, srcLineSize);
    }

    ANativeWindow_unlockAndPost(mNativeWindow);
}