package com.example.helloffmpeg

import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.example.helloffmpeg.media.MediaPlayer
import com.example.helloffmpeg.media.VIDEO_RENDER_ANWINDOW

private const val TAG = "lorien"

class NativeRendActivity : AppCompatActivity(), SurfaceHolder.Callback {

    private var videoPath = ""

    private lateinit var surfaceView: SurfaceView
    private lateinit var seekBar: SeekBar

    private var isTouch = false

    private var mediaPlayer: MediaPlayer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_native_rend)

        videoPath = getExternalFilesDir(null)?.absolutePath + "/lorien/" + "huoying.mp4"

        surfaceView = findViewById(R.id.surface_view)
        surfaceView.holder.addCallback(this)

        seekBar = findViewById(R.id.seek_bar)
        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                isTouch = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                Log.d(TAG, "onStopTrackingTouch called with progress=${seekBar?.progress}")
            }

        })
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        Log.d(TAG, "surfaceCreated")

        mediaPlayer = MediaPlayer()
        // 初始化播放器
        mediaPlayer?.init(videoPath, VIDEO_RENDER_ANWINDOW, holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        Log.d(TAG, "surfaceChanged, width=$width, height=$height")
        mediaPlayer?.play()
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        Log.d(TAG, "surfaceDestroyed")
        mediaPlayer?.stop()
        mediaPlayer?.unInit()
    }
}