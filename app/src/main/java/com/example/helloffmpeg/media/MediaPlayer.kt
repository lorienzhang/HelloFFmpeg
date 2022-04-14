package com.example.helloffmpeg.media

import android.view.Surface

/** 画面渲染-ANativeWindow，要和native的渲染type值对应上，查看：VideoRender.h */
const val VIDEO_RENDER_ANWINDOW = 0

/** msg */
const val MSG_DECODER_INIT_ERROR = 0
const val MSG_DECODER_READY = 1
const val MSG_DECODER_DONE = 2
const val MSG_REQUEST_RENDER = 3
const val MSG_DECODING_TIME = 4

/** param */
const val MEDIA_PARAM_VIDEO_WIDTH = 0x001
const val MEDIA_PARAM_VIDEO_HEIGHT = 0x002
const val MEDIA_PARAM_VIDEO_DURATION = 0x003

class MediaPlayer {
    companion object {
        init {
            System.loadLibrary("hello-ffmpeg")
        }

        fun getFFmpegVersion(): String {
            return nativeGetFFmpegVersion()
        }
    }

    private external fun nativeInit(url: String, videoRenderType: Int, surface: Surface): Long
    private external fun nativePlay(playerHandle: Long)
    private external fun nativeStop(playerHandle: Long)
    private external fun nativeUnInit(playerHandle: Long)
    private external fun nativeGetMediaParams(playerHandle: Long, paramType: Int): Long
    private external fun nativeSeekToPosition(playerHandle: Long, position: Float)

    /** native播放器地址 */
    private var nativePlayerHandle = 0L
    private var eventCallback: EventCallback? = null

    /**
     * 初始化播放器
     *
     * @param url 视频url
     * @param videoRenderType 播放画面渲染类型
     * @param surface
     */
    fun init(url: String, videoRenderType: Int, surface: Surface) {
        nativePlayerHandle = nativeInit(url, videoRenderType, surface)
    }

    fun unInit() {
        nativeUnInit(nativePlayerHandle)
    }

    fun play() {
        nativePlay(nativePlayerHandle)
    }

    fun stop() {
        nativeStop(nativePlayerHandle)
    }

    fun getMediaParams(paramType: Int): Long {
        return nativeGetMediaParams(nativePlayerHandle, paramType)
    }

    fun seekToPosition(position: Float) {
        return nativeSeekToPosition(nativePlayerHandle, position)
    }

    /**
     * native通过该方法向java回调播放器相关状态
     */
    private fun playerEventCallback(msgType: Int, msgValue: Float) {
        eventCallback?.onPlayerEvent(msgType, msgValue)
    }

    fun addEventCallback(callback: EventCallback) {
        this.eventCallback = callback
    }
}

interface EventCallback {
    fun onPlayerEvent(msgType: Int, msgValue: Float)
}

external fun nativeGetFFmpegVersion(): String
