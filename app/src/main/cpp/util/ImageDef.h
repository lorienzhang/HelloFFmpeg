#ifndef HELLOFFMPEG_IMAGEDEF_H
#define HELLOFFMPEG_IMAGEDEF_H

#include <unistd.h>

#define IMAGE_FORMAT_RGBA           0x01

typedef struct _tag_NativeImage {
    int width;
    int height;
    int format;
    uint8_t *ppPlane[3];
    int pLineSize[3];

    _tag_NativeImage() {
        width = 0;
        height = 0;
        format = 0;
        ppPlane[0] = nullptr;
        ppPlane[1] = nullptr;
        ppPlane[2] = nullptr;
        pLineSize[0] = 0;
        pLineSize[1] = 0;
        pLineSize[2] = 0;
    }
} NativeImage;

#endif //HELLOFFMPEG_IMAGEDEF_H
