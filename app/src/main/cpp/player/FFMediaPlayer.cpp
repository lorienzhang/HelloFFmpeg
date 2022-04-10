#include "FFMediaPlayer.h"
#include "../util/LogUtil.h"

void FFMediaPlayer::init(JNIEnv *env, jobject obj, char *url, int renderType, jobject surface) {
    env->GetJavaVM(&mJavaVM);
    // 存储调用的java对象
    mJavaObj = env->NewGlobalRef(obj);

    mVideoDecoder = new VideoDecoder(url);
    mVideoDecoder->setMessageCallback(this, postMessage);
}

void FFMediaPlayer::unInit() {
    LOGD("FFMediaPlayer::unInit");
    if (mVideoDecoder) {
        delete mVideoDecoder;
        mVideoDecoder = nullptr;
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
        mVideoDecoder->stop();
    }
}

void FFMediaPlayer::seekToPosition(float position) {
    LOGD("FFMediaPlayer::seekToPosition");
    if (mVideoDecoder) {
        mVideoDecoder->seekToPosition(position);
    }
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
        LOGD("FFMediaPlayer::postMessage evn=%p",env);
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