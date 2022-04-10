#include "BaseDecoder.h"
#include "../../util/LogUtil.h"

void BaseDecoder::start() {
    if (mThread == nullptr) {
        startDecodingThread();
    } else {
        // 唤醒解码线程
        std::unique_lock<std::mutex> lock(mMutex);
        mDecoderState = STATE_DECODING;
        mCond.notify_all();
    }
}

void BaseDecoder::pause() {
    std::unique_lock<std::mutex> lock(mMutex);
    mDecoderState = STATE_PAUSE;
}

void BaseDecoder::stop() {
    std::unique_lock<std::mutex> lock(mMutex);
    mDecoderState = STATE_STOP;
    mCond.notify_all();
}

void BaseDecoder::seekToPosition(float position) {

}

float BaseDecoder::getCurrentPosition() {
    return mCurTimeStamp;
}

void BaseDecoder::startDecodingThread() {
    mThread = new thread(doAVDecode, this);
}

int BaseDecoder::init(const char *url, AVMediaType type) {
    LOGD("BaseDecoder::init");
    strcpy(mUrl, url);
    mMediaType = type;
    return 0;
}

void BaseDecoder::unInit() {
    LOGD("BaseDecoder::unInit, mMediaType=%d\n", mMediaType);
    if (mThread) {
        stop();
        mThread->join();
        delete mThread;
        mThread = nullptr;
    }
    LOGD("BaseDecoder::unInit end, mMediaType=%d\n", mMediaType);
}

/**
 * 初始化FFmpeg解码器
 *
 * @return 成功返回0，失败返回-1
 */
int BaseDecoder::initFFDecoder() {
    int res = -1;
    do {
        // 1. 创建封装格式上下文
        mAVFormatContext = avformat_alloc_context();

        // 2. 打开文件
        if (avformat_open_input(&mAVFormatContext, mUrl, NULL, NULL) != 0) {
            LOGD("BaseDecoder::initFFDecoder, avformat_open_input fail\n");
            break;
        }

        // 3. 获取音视频信息流信息
        if (avformat_find_stream_info(mAVFormatContext, NULL) < 0) {
            LOGD("BaseDecoder::initFFDecoder, avformat_find_stream_info fail\n");
            break;
        }

        // 4. 获取音视频流的索引
        for (int i = 0; i < mAVFormatContext->nb_streams; i++) {
            if (mAVFormatContext->streams[i]->codecpar->codec_type == mMediaType) {
                mStreamIndex = i;
                break;
            }
        }

        if (mStreamIndex == -1) {
            LOGD("BaseDecoder::initFFDecoder, failed to find stream index\n");
            break;
        }

        // 5. 获取解码器参数
        AVCodecParameters *codecParameters = mAVFormatContext->streams[mStreamIndex]->codecpar;

        // 6. 获取解码器
        mAVCodec = avcodec_find_decoder(codecParameters->codec_id);
        if (mAVCodec == nullptr) {
            LOGD("BaseDecoder::initFFDecoder, avcodec_find_decoder fail\n");
            break;
        }

        // 7. 创建解码器上下文
        mAVCodecContext = avcodec_alloc_context3(mAVCodec);
        if (avcodec_parameters_to_context(mAVCodecContext, codecParameters) != 0) {
            LOGD("BaseDecoder::initFFDecoder, avcodec_parameters_to_context fail\n");
            break;
        }

        // 8. open 解码器
        AVDictionary *pAVDictionary = nullptr;
        av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
        av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
        av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
        av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);
        res = avcodec_open2(mAVCodecContext, mAVCodec, &pAVDictionary);
        if (res < 0) {
            LOGD("BaseDecoder::initFFDecoder, avcodec_open2 fail, result=%d\n", res);
            break;
        }

        res = 0;
        //us to ms
        mDuration = mAVFormatContext->duration / AV_TIME_BASE * 1000;
        // 分配编码帧对象
        mAVPacket = av_packet_alloc();
        // 分配解码后帧对象
        mFrame = av_frame_alloc();
    } while (false);

    if (res != 0 && mMsgContext && mMsgCallback) {
        mMsgCallback(mMsgContext, MSG_DECODER_INIT_ERROR, 0);
    }

    return res;
}

void BaseDecoder::unInitDecoder() {
    LOGD("BaseDecoder::unInitDecoder");
    if (mFrame != nullptr) {
        av_frame_free(&mFrame);
        mFrame = nullptr;
    }

    if (mAVPacket != nullptr) {
        av_packet_free(&mAVPacket);
        mAVPacket = nullptr;
    }

    if (mAVCodecContext != nullptr) {
        avcodec_close(mAVCodecContext);
        avcodec_free_context(&mAVCodecContext);
        mAVCodecContext = nullptr;
        mAVCodec = nullptr;
    }

    if (mAVFormatContext != nullptr) {
        avformat_close_input(&mAVFormatContext);
        avformat_free_context(mAVFormatContext);
        mAVFormatContext = nullptr;
    }
}

void BaseDecoder::decodingLoop() {
    LOGD("BaseDecoder::decodingLoop start, mMediaType=%d\n", mMediaType);
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mDecoderState = STATE_DECODING;
        lock.unlock();
    }

    for (;;) {
        while (mDecoderState == STATE_PAUSE) {
            // 暂定状态，条件等待
            std::unique_lock<std::mutex> lock(mMutex);
            LOGD("BaseDecoder::decodingLoop waiting, mMediaType=%d\n", mMediaType);
            mCond.wait_for(lock, std::chrono::milliseconds(10));
            // 当前时间 - 当前播放时间
            mStartTimeStamp = getSysCurrentTime() - mCurTimeStamp;
        }

        if (mDecoderState == STATE_STOP) {
            break;
        }

        if (mStartTimeStamp == -1) {
            // 设置起播时间
            mStartTimeStamp = getSysCurrentTime();
        }

        if (decodeOnePacket() != 0) {
            // 解码结束，修改状态
            std::unique_lock<std::mutex> lock(mMutex);
            mDecoderState = STATE_PAUSE;
        }
    }

    LOGD("BaseDecoder::decodingLoop end\n");
}

int BaseDecoder::decodeOnePacket() {
    LOGD("BaseDecoder::decodeOnePacket mMediaType=%d\n", mMediaType);
    if (mSeekPosition > 0) {
        // todo: seek
    }

    int result = av_read_frame(mAVFormatContext, mAVPacket);
    while (result == 0) {
        if (mAVPacket->stream_index == mStreamIndex) {
            // packet发送给解码器解码
            if (avcodec_send_packet(mAVCodecContext, mAVPacket) == AVERROR_EOF) {
                // 解码出错
                result = -1;
                goto __EXIT;
            }

            int frameCount = 0;
            while (avcodec_receive_frame(mAVCodecContext, mFrame)) {
                // 解完一帧，更新时间戳
                updateTimeStamp();
                // 音视频同步
                AVSync();
                // 渲染
                onFrameAvailable(mFrame);
                frameCount++;
            }
            LOGD("BaseDecoder::decodeOnePacket frameCount=%d\n", frameCount);

            if (frameCount > 0) {
                // 解码顺利完成
                result = 0;
                goto __EXIT;
            }
        }
        av_packet_unref(mAVPacket);
        result = av_read_frame(mAVFormatContext, mAVPacket);
    }

__EXIT:
    av_packet_unref(mAVPacket);
    return result;
}

/**
 * 音视频同步，像系统时间对齐
 * @return
 */
long BaseDecoder::AVSync() {
    LOGD("BaseDecoder::AVSync");
    // ms
    long curSysTime = getSysCurrentTime();
    // 距离起播时间的流逝时间
    long elapsedTime = curSysTime - mStartTimeStamp;
    // 一帧对外回调
    if (mMsgContext && mMsgCallback && mMediaType == AVMEDIA_TYPE_AUDIO) {
        // 回调当前播放时间戳，转成秒
        mMsgCallback(mMsgContext, MSG_DECODING_TIME, mCurTimeStamp * 1.0f / 1000);
    }

    long delay = 0;
    if (mCurTimeStamp > elapsedTime) {
        auto sleepTime = static_cast<unsigned int >(mCurTimeStamp - elapsedTime);
        // 限制休眠时间
        sleepTime = sleepTime > DELAY_THRESHOLD ? DELAY_THRESHOLD : sleepTime;
        LOGD("BaseDecoder::AVSync, sleepTime=%d", sleepTime);
        av_usleep(sleepTime * 1000);
    }
    delay = elapsedTime - mCurTimeStamp;
    return delay;
}

/**
 * 更新当前播放时间戳
 */
void BaseDecoder::updateTimeStamp() {
    LOGD("BaseDecoder::updateTimeStamp");
    if (mFrame->pkt_dts != AV_NOPTS_VALUE) {
        mCurTimeStamp = mFrame->pkt_dts;
    } else if (mFrame->pts != AV_NOPTS_VALUE) {
        mCurTimeStamp = mFrame->pts;
    } else {
        mCurTimeStamp = 0;
    }

    // ms
    mCurTimeStamp = (int64_t) (
            (mCurTimeStamp * av_q2d(mAVFormatContext->streams[mStreamIndex]->time_base)) * 1000);
    LOGD("BaseDecoder::updateTimeStamp, mCurTimeStamp=%d\n", mCurTimeStamp);
}

/**
 * 解码线程执行函数
 *
 * @param decoder
 */
void BaseDecoder::doAVDecode(BaseDecoder *decoder) {
    LOGD("BaseDecoder::doAVDecode");
    do {
        // 1. 初始化FFmpeg解码器
        if (decoder->initFFDecoder() != 0) {
            // 初始化ffmpeg解码器失败
            break;
        }
        // 2. 循环解码
        decoder->onDecoderReady();
        decoder->decodingLoop();
    } while (false);

    decoder->unInitDecoder();
    decoder->onDecoderDone();
}