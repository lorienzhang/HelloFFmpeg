package com.example.helloffmpeg

import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.example.helloffmpeg.media.*

private const val TAG = "lorien"

class NativeRenderActivity : AppCompatActivity(), SurfaceHolder.Callback, EventCallback {

    private var videoPath = ""

    private lateinit var surfaceView: SizeAdapterSurfaceView
    private lateinit var seekBar: SeekBar

    private var isTouch = false

    private var mediaPlayer: MediaPlayer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_native_render)

        videoPath = getExternalFilesDir(null)?.absolutePath + "/lorien/" + "haizei.mp4"

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
                mediaPlayer?.let {
                    it.seekToPosition(seekBar!!.progress.toFloat())
                    isTouch = false
                }
            }
        })
    }

    private fun onDecoderReady() {
        val videoWidth = mediaPlayer?.getMediaParams(MEDIA_PARAM_VIDEO_WIDTH)
        val videoHeight = mediaPlayer?.getMediaParams(MEDIA_PARAM_VIDEO_HEIGHT)
        if (videoWidth!! * videoHeight!! != 0L) {
            surfaceView.setAspectRatio(videoWidth.toInt(), videoHeight.toInt())
        }

        val duration = mediaPlayer?.getMediaParams(MEDIA_PARAM_VIDEO_DURATION)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            seekBar.min = 0
        }
        seekBar.max = duration?.toInt()!!
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        Log.d(TAG, "surfaceCreated")

        mediaPlayer = MediaPlayer()
        mediaPlayer?.addEventCallback(this)
        // 初始化播放器
        mediaPlayer?.init(videoPath, VIDEO_RENDER_ANWINDOW, holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        Log.d(TAG, "surfaceChanged, width=$width, height=$height")
        // native会启动线程，执行解码、渲染工作
        mediaPlayer?.play()
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        Log.d(TAG, "surfaceDestroyed")
        // 停止播放，释放资源
        mediaPlayer?.stop()
        mediaPlayer?.unInit()
    }

    override fun onPlayerEvent(msgType: Int, msgValue: Float) {
        Log.d(TAG, "onPlayerEvent, msgType=$msgType, msgValue=$msgValue")
        runOnUiThread {
            when (msgType) {
                MSG_DECODER_READY -> {
                    // 调整surfaceview的size
                    onDecoderReady()
                }

                MSG_DECODING_TIME -> {
                    if (!isTouch) {
                        seekBar.progress = msgValue.toInt()
                    }
                }
            }
        }
    }
}