#ifndef HELLOFFMPEG_NATIVERENDER_H
#define HELLOFFMPEG_NATIVERENDER_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>

#include "VideoRender.h"

class NativeRender : public VideoRender {

public:
    NativeRender(JNIEnv *env, jobject surface);

    virtual ~NativeRender();

    virtual void init(int videoWidth, int videoHeight, int *dstSize);

    virtual void unInit();

    virtual void renderVideoFrame(NativeImage *pImage);

private:
    ANativeWindow_Buffer mNativeWindowBuffer;
    ANativeWindow *mNativeWindow = nullptr;
    int mDstWidth;
    int mDstHeight;
};

#endif //HELLOFFMPEG_NATIVERENDER_H
