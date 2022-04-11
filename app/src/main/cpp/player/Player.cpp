#include "Player.h"
#include "FFMediaPlayer.h"

void Player::init(JNIEnv *jniEnv, jobject obj, char *url, int renderType, jobject surface) {
    mMediaPlayer = new FFMediaPlayer();

    if (mMediaPlayer) {
        mMediaPlayer->init(jniEnv, obj, url, renderType, surface);
    }
}

void Player::unInit() {
    if (mMediaPlayer) {
        mMediaPlayer->unInit();
        delete mMediaPlayer;
        mMediaPlayer = nullptr;
    }
}

void Player::play() {
    if (mMediaPlayer) {
        mMediaPlayer->play();
    }
}

void Player::pause() {
    if (mMediaPlayer) {
        mMediaPlayer->pause();
    }
}

void Player::stop() {
    if (mMediaPlayer) {
        mMediaPlayer->stop();
    }
}

void Player::seekToPosition(float position) {
    if (mMediaPlayer) {
        mMediaPlayer->seekToPosition(position);
    }
}

long Player::getMediaParams(int paramType) {
    if (mMediaPlayer) {
        return mMediaPlayer->getMediaParams(paramType);
    }
    return 0;
}