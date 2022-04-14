#include <jni.h>
#include <string>
#include "player/Player.h"

#include "util/LogUtil.h"

extern "C" {
#include "libavcodec/version.h"
#include "libavcodec/avcodec.h"
#include "libavformat/version.h"
#include "libavutil/version.h"
#include "libavfilter/version.h"
#include "libswresample/version.h"
#include "libswscale/version.h"
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_helloffmpeg_media_MediaPlayerKt_nativeGetFFmpegVersion(
        JNIEnv* env,
        jclass clazz) {

    char strBuffer[1024 * 4] = {0};
    strcat(strBuffer, "libavcodec : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(strBuffer, "\nlibavformat : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(strBuffer, "\nlibavutil : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(strBuffer, "\nlibavfilter : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(strBuffer, "\nlibswresample : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(strBuffer, "\nlibswscale : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWSCALE_VERSION));
    strcat(strBuffer, "\navcodec_config : ");
    strcat(strBuffer, avcodec_configuration());
    strcat(strBuffer, "\navcodec_license : ");
    strcat(strBuffer, avcodec_license());
    LOGD("GetFFmpegVersion\n%s", strBuffer);

    return env->NewStringUTF(strBuffer);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_helloffmpeg_media_MediaPlayer_nativeInit(
        JNIEnv* env,
        jobject obj,
        jstring jurl,
        jint renderType,
        jobject surface) {
    const char *url = env->GetStringUTFChars(jurl, nullptr);
    Player *player = new Player();
    player->init(env, obj, const_cast<char *>(url), renderType, surface);
    env->ReleaseStringUTFChars(jurl, url);
    return reinterpret_cast<jlong>(player);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloffmpeg_media_MediaPlayer_nativePlay(
        JNIEnv* env,
        jobject obj,
        jlong player_handle) {
    if (player_handle != 0) {
        Player *player = reinterpret_cast<Player *>(player_handle);
        player->play();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloffmpeg_media_MediaPlayer_nativeStop(
        JNIEnv* env,
        jobject obj,
        jlong player_handle) {
    if (player_handle != 0) {
        Player *player = reinterpret_cast<Player *>(player_handle);
        player->stop();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloffmpeg_media_MediaPlayer_nativeUnInit(
        JNIEnv* env,
        jobject obj,
        jlong player_handle) {
    if (player_handle != 0) {
        Player *player = reinterpret_cast<Player *>(player_handle);
        player->unInit();
    }
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_example_helloffmpeg_media_MediaPlayer_nativeGetMediaParams(
        JNIEnv* env,
        jobject obj,
        jlong player_handle,
        jint param_type) {
    long value = 0;
    if (player_handle != 0) {
        Player *player = reinterpret_cast<Player *>(player_handle);
        value = player->getMediaParams(param_type);
    }
    return value;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloffmpeg_media_MediaPlayer_nativeSeekToPosition(
        JNIEnv* env,
        jobject obj,
        jlong player_handle,
        jfloat position) {
    if (player_handle != 0) {
        Player *player = reinterpret_cast<Player *>(player_handle);
        player->seekToPosition(position);
    }
}