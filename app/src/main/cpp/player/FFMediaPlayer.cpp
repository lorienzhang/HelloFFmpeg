#include "render/video/NativeRender.h"

#include "FFMediaPlayer.h"
#include "../util/LogUtil.h"

void FFMediaPlayer::init(JNIEnv *env, jobject obj, char *url, int renderType, jobject surface) {
    env->GetJavaVM(&mJavaVM);
    // 存储调用的java对象
    mJavaObj = env->NewGlobalRef(obj);

    // 构造视频解码器
    mVideoDecoder = new VideoDecoder(url);
    if (renderType == VIDEO_RENDER_ANWINDOW) {
        // 构造视频渲染
        mVideoRender = new NativeRender(env, surface);
        // 关联VideoDecoder&VideoRender
        mVideoDecoder->setVideoRender(mVideoRender);
    }

    mVideoDecoder->setMessageCallback(this, postMessage);
}

void FFMediaPlayer::unInit() {
    LOGD("FFMediaPlayer::unInit");
    if (mVideoDecoder) {
        delete mVideoDecoder;
        mVideoDecoder = nullptr;
    }

    if (mVideoRender) {
        delete mVideoRender;
        mVideoRender = nullptr;
    }

    bool isAttach = false;
    getJNIEnv(&isAttach)->DeleteGlobalRef(mJavaObj);
    if (isAttach) {
        getJavaVM()->DetachCurrentThread();
    }
}

void FFMediaPlayer::play() {
    LOGD("FFMediaPlayer::play");
    if (mVideoDecoder) {
        mVideoDecoder->start();
    }
}

void FFMediaPlayer::pause() {
    LOGD("FFMediaPlayer::pause");
    if (mVideoDecoder) {
        mVideoDecoder->pause();
    }
}

void FFMediaPlayer::stop() {
    LOGD("FFMediaPlayer::stop");
    if (mVideoDecoder) {
        // stop解码器
        mVideoDecoder->stop();
    }
}

void FFMediaPlayer::seekToPosition(float position) {
    LOGD("FFMediaPlayer::seekToPosition");
    if (mVideoDecoder) {
        mVideoDecoder->seekToPosition(position);
    }
}

long FFMediaPlayer::getMediaParams(int paramType) {
    LOGD("FFMediaPlayer::getMediaParams paramType=%d\n", paramType);
    long value = 0;
    switch (paramType) {
        case MEDIA_PARAM_VIDEO_WIDTH:
            value = mVideoDecoder != nullptr ? mVideoDecoder->getVideoWidth() : 0;
            break;
        case MEDIA_PARAM_VIDEO_HEIGHT:
            value = mVideoDecoder != nullptr ? mVideoDecoder->getVideoHeight() : 0;
            break;
        case MEDIA_PARAM_VIDEO_DURATION:
            value = mVideoDecoder != nullptr ? mVideoDecoder->getDuration() : 0;
            break;
    }
    return value;
}

JNIEnv *FFMediaPlayer::getJNIEnv(bool *isAttach) {
    JNIEnv *env;
    int status;
    if (mJavaVM == nullptr) {
        LOGD("FFMediaPlayer::getJNIEnv, mJavaVM == nullptr");
        return nullptr;
    }

    *isAttach = false;
    status = mJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status != JNI_OK) {
        status = mJavaVM->AttachCurrentThread(&env, nullptr);
        if (status != JNI_OK) {
            LOGD("FFMediaPlayer::getJNIEnv, AttachCurrentThread failed");
            return nullptr;
        }
        *isAttach = true;
    }

    return env;
}

jobject FFMediaPlayer::getJavaObj() {
    return mJavaObj;
}

JavaVM *FFMediaPlayer::getJavaVM() {
    return mJavaVM;
}


void FFMediaPlayer::postMessage(void *context, int msgType, float msgCode) {
    if (context != nullptr) {
        FFMediaPlayer *player = static_cast<FFMediaPlayer *>(context);
        bool isAttach = false;
        JNIEnv *env = player->getJNIEnv(&isAttach);
        LOGD("FFMediaPlayer::postMessage msgType=%d, JNIEnv=%p",msgType, env);
        if (env == nullptr) {
            return;
        }
        jobject javaObj = player->getJavaObj();
        jmethodID mid = env->GetMethodID(env->GetObjectClass(javaObj),
                                         JAVA_PLAYER_EVENT_CALLBACK_API_NAME, "(IF)V");
        env->CallVoidMethod(javaObj, mid, msgType, msgCode);
        if (isAttach) {
            player->getJavaVM()->DetachCurrentThread();
        }
    }
}