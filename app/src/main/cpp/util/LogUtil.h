#ifndef HELLOFFMPEG_LOGUTIL_H
#define HELLOFFMPEG_LOGUTIL_H

#include "android/log.h"
#include <sys/time.h>

#define LOG_TAG "LorienNative"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

static long long getSysCurrentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    long long curTime = ((long long) (time.tv_sec)) * 1000 + time.tv_usec / 1000;
    return curTime;
}

#endif //HELLOFFMPEG_LOGUTIL_H
