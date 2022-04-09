package com.example.helloffmpeg.media

class FFMediaPlayer {
    companion object {
        init {
            System.loadLibrary("hello-ffmpeg")
        }

        fun getFFmpegVersion(): String {
            return nativeGetFFmpegVersion()
        }
    }
}

external fun nativeGetFFmpegVersion(): String
