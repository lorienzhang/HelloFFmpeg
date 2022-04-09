#ifndef HELLOFFMPEG_LOGUTIL_H
#define HELLOFFMPEG_LOGUTIL_H

#include "android/log.h"

#define LOG_TAG "LorienNative"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

#endif //HELLOFFMPEG_LOGUTIL_H
