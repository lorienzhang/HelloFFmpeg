#ifndef HELLOFFMPEG_OPENSLRENDER_H
#define HELLOFFMPEG_OPENSLRENDER_H

#include <cstdint>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <queue>
#include <string>
#include <thread>
#include "AudioRender.h"

class OpenSLRender : public AudioRender {
public:
    OpenSLRender() {}
    virtual void init();
    virtual void unInit();
    virtual void renderAudioFrame(uint8_t *pData, int dataSize);
    virtual void clearAudioCache();

private:
    int createEngine();
    int createOutputMixer();
    int createAudioPlayer();
    int getAudioFrameQueueSize();
    void startRender();
    void handleAudioFrameQueue();

    static void createSLWaitingThread(OpenSLRender *openSlRender);
    static void audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    SLObjectItf mEngineObj = nullptr;
    SLEngineItf mEngineEngine = nullptr;
    SLObjectItf mOutputMixObj = nullptr;
    SLObjectItf mAudioPlayerObj = nullptr;
    SLPlayItf mAudioPlayerPlay = nullptr;
    SLVolumeItf mAudioPlayerVolume = nullptr;
    SLAndroidSimpleBufferQueueItf mBufferQueue;

    std::queue<AudioFrame *> mAudioFrameQueue;

    std::thread *mThread = nullptr;
    std::mutex mMutex;
    std::condition_variable mCond;
    volatile bool mExit = false;
};

#endif //HELLOFFMPEG_OPENSLRENDER_H
