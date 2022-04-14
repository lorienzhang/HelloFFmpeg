#include "OpenSLRender.h"
#include "../util/LogUtil.h"

/**
 * 初始化OpenSL
 */
void OpenSLRender::init() {
    LOGD("OpenSLRender::init");

    int result = -1;
    do {
        result = createEngine();
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::init createEngine fail, result=%d", result);
            break;
        }

        result = createOutputMixer();
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::init createOutputMixer fail, result=%d", result);
            break;
        }

        result = createAudioPlayer();
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createEngine GetInterface fail. result=%d\n", result);
            break;
        }

        mThread = new std::thread(createSLWaitingThread, this);

    } while (false);

    if (result != SL_RESULT_SUCCESS) {
        LOGD("OpenSLRender::init fail result=%d\n", result);
        unInit();
    }
}

void OpenSLRender::unInit() {
    LOGD("OpenSLRender::unInit");

    if (mAudioPlayerPlay) {
        (*mAudioPlayerPlay)->SetPlayState(mAudioPlayerPlay, SL_PLAYSTATE_STOPPED);
        mAudioPlayerPlay = nullptr;
    }

    std::unique_lock<std::mutex> lock(mMutex);
    mExit = true;
    mCond.notify_all();
    lock.unlock();

    if (mAudioPlayerObj) {
        (*mAudioPlayerObj)->Destroy(mAudioPlayerObj);
        mAudioPlayerObj = nullptr;
        mBufferQueue = nullptr;
    }

    if (mOutputMixObj) {
        (*mOutputMixObj)->Destroy(mOutputMixObj);
        mOutputMixObj = nullptr;
    }

    if (mEngineObj) {
        (*mEngineObj)->Destroy(mEngineObj);
        mEngineObj = nullptr;
        mEngineEngine = nullptr;
    }

    lock.lock();
    for (int i = 0; i < mAudioFrameQueue.size(); ++i) {
        AudioFrame *audioFrame = mAudioFrameQueue.front();
        mAudioFrameQueue.pop();
        delete audioFrame;
    }
    lock.unlock();

    if (mThread != nullptr) {
        mThread->join();
        delete mThread;
        mThread = nullptr;
    }
}

int OpenSLRender::createEngine() {
    SLresult result = SL_RESULT_SUCCESS;
    do {
        result = slCreateEngine(&mEngineObj, 0, nullptr,
                                0, nullptr, nullptr);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createEngine slCreateEngine fail. result=%d\n", result);
            break;
        }

        result = (*mEngineObj)->Realize(mEngineObj, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createEngine Realize fail. result=%d\n", result);
            break;
        }

        result = (*mEngineObj)->GetInterface(mEngineObj, SL_IID_ENGINE, &mEngineEngine);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createEngine GetInterface fail. result=%d\n", result);
            break;
        }

    } while (false);

    return result;
}

int OpenSLRender::createOutputMixer() {
    SLresult result = SL_RESULT_SUCCESS;
    do {
        const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
        const SLboolean mreg[1] = {SL_BOOLEAN_FALSE};

        result = (*mEngineEngine)->CreateOutputMix(mEngineEngine, &mOutputMixObj, 1, mids, mreg);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createOutputMixer CreateOutputMix fail. result=%d\n", result);
            break;
        }

        result = (*mOutputMixObj)->Realize(mOutputMixObj, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createOutputMixer Realize fail. result=%d\n", result);
            break;
        }
    } while (false);

    return result;
}

int OpenSLRender::createAudioPlayer() {
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM, // format type
            (SLuint32)2, // channel count
            SL_SAMPLINGRATE_44_1, // 44100HZ
            SL_PCMSAMPLEFORMAT_FIXED_16, // bits per sample
            SL_PCMSAMPLEFORMAT_FIXED_16, // container size
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // channel mask
            SL_BYTEORDER_LITTLEENDIAN // 小端字节
    };
    SLDataSource slDataSource = {&android_queue, &pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mOutputMixObj};
    SLDataSink slDataSink = {&outputMix, nullptr};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    SLresult result;

    do {
        result = (*mEngineEngine)->CreateAudioPlayer(mEngineEngine, &mAudioPlayerObj, &slDataSource,
                                                     &slDataSink, 3, ids, req);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createAudioPlayer CreateAudioPlayer fail. result=%d\n", result);
            break;
        }

        result = (*mAudioPlayerObj)->Realize(mAudioPlayerObj, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createAudioPlayer Realize mAudioPlayerObj fail. result=%d\n",
                 result);
            break;
        }

        result = (*mAudioPlayerObj)->GetInterface(mAudioPlayerObj, SL_IID_PLAY, &mAudioPlayerPlay);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createAudioPlayer GetInterface SL_IID_PLAY fail. result=%d\n",
                 result);
            break;
        }

        result = (*mAudioPlayerObj)->GetInterface(mAudioPlayerObj, SL_IID_BUFFERQUEUE,
                                                  &mBufferQueue);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createAudioPlayer GetInterface SL_IID_BUFFERQUEUE fail. result=%d\n",
                 result);
            break;
        }

        // 当OpenSL ES需要数据进行播放的时候会回调该函数，我们要在回调函数中填充PCM数据
        result = (*mBufferQueue)->RegisterCallback(mBufferQueue, audioPlayerCallback, this);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createAudioPlayer RegisterCallback fail. result=%d\n", result);
            break;
        }

        result = (*mAudioPlayerObj)->GetInterface(mAudioPlayerObj, SL_IID_VOLUME,
                                                  &mAudioPlayerVolume);
        if (result != SL_RESULT_SUCCESS) {
            LOGD("OpenSLRender::createAudioPlayer GetInterface SL_IID_VOLUME fail. result=%d\n",
                 result);
            break;
        }

    } while (false);

    return result;
}

/**
 * 每解出一帧，调用该方法渲染。
 *
 * 将一帧数据压入队列
 *
 * @param pData
 * @param dataSize
 */
void OpenSLRender::renderAudioFrame(uint8_t *pData, int dataSize) {
    LOGD("OpenSLRender::renderAudioFrame pData=%p, dataSize=%d\n", pData, dataSize);
    if (mAudioPlayerPlay) {
        if (pData != nullptr && dataSize > 0) {
            while (getAudioFrameQueueSize() >= MAX_QUEUE_BUFFER_SIZE && !mExit) {
                // 队列里的帧数据过多，等会...
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }

            std::unique_lock<std::mutex> lock(mMutex);
            AudioFrame *audioFrame = new AudioFrame(pData, dataSize);
            mAudioFrameQueue.push(audioFrame);
            mCond.notify_all();
            lock.unlock();
        }
    }
}

void OpenSLRender::startRender() {
    LOGD("OpenSLRender::startRender");
    // 等队列中的音频帧的数量达到MAX_QUEUE_BUFFER_SIZE，才开始播放音频
    while (getAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE && !mExit) {
        std::unique_lock<std::mutex> lock(mMutex);
        mCond.wait_for(lock, std::chrono::milliseconds(10));
        lock.unlock();
    }

    // 设置音频为播放状态
    (*mAudioPlayerPlay)->SetPlayState(mAudioPlayerPlay, SL_PLAYSTATE_PLAYING);
    audioPlayerCallback(mBufferQueue, this);
}

/**
 * 向SLAndroidSimpleBufferQueueItf队列中放入一帧数据
 */
void OpenSLRender::handleAudioFrameQueue() {
    LOGD("OpenSLRender::handleAudioFrameQueue frameQueueSize=%ld\n", mAudioFrameQueue.size());
    if (mAudioPlayerPlay == nullptr) {
        return;
    }

    while (getAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE && !mExit) {
        std::unique_lock<std::mutex> lock(mMutex);
        mCond.wait_for(lock, std::chrono::milliseconds(10));
    }

    std::unique_lock<std::mutex> lock(mMutex);
    AudioFrame *audioFrame = mAudioFrameQueue.front();
    if (audioFrame != nullptr && mAudioPlayerPlay) {
        // 将音频PCM帧提供给SL
        SLresult result = (*mBufferQueue)->Enqueue(mBufferQueue, audioFrame->data,
                                                   (SLuint32) audioFrame->dataSize);
        if (result == SL_RESULT_SUCCESS) {
            // 出队列，销毁已经播放过的音频帧
            mAudioFrameQueue.pop();
            delete audioFrame;
        }
    }
    lock.unlock();
}

/**
 * 线程函数体，等待线程，队列中有3帧数据，才开始开启OpenSL渲染
 *
 * @param openSlRender
 */
void OpenSLRender::createSLWaitingThread(OpenSLRender *openSlRender) {
    // 启动渲染
    openSlRender->startRender();
}

void OpenSLRender::audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
    OpenSLRender *openSlRender = static_cast<OpenSLRender *>(context);
    openSlRender->handleAudioFrameQueue();
}

int OpenSLRender::getAudioFrameQueueSize() {
    std::unique_lock<std::mutex> lock(mMutex);
    return mAudioFrameQueue.size();
}

void OpenSLRender::clearAudioCache() {
    std::unique_lock<std::mutex> lock(mMutex);
    for (int i = 0; i < mAudioFrameQueue.size(); ++i) {
        AudioFrame *audioFrame = mAudioFrameQueue.front();
        mAudioFrameQueue.pop();
        delete audioFrame;
    }
}