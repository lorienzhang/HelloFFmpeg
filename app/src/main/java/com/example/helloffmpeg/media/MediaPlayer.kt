package com.example.helloffmpeg.media

import android.view.Surface

/** 画面渲染-ANativeWindow */
const val VIDEO_RENDER_ANWINDOW = 1

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

    /** native播放器地址 */
    private var nativePlayerHandle = 0L

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

    /**
     * native通过该方法向java回调播放器相关状态
     */
    private fun playerEventCallback(msgType: Int, msgValue: Float) {

    }
}

external fun nativeGetFFmpegVersion(): String
