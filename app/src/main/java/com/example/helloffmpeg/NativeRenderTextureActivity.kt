package com.example.helloffmpeg

import android.graphics.Matrix
import android.graphics.SurfaceTexture
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.Surface
import android.view.TextureView
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.example.helloffmpeg.media.*

private const val TAG = "lorien"

class NativeRenderTextureActivity : AppCompatActivity(), EventCallback,
    TextureView.SurfaceTextureListener {

    private var videoPath = ""

    private lateinit var textureView: TextureView
    private lateinit var seekBar: SeekBar

    private var isTouch = false

    private var mediaPlayer: MediaPlayer? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_native_texture_render)

        videoPath = getExternalFilesDir(null)?.absolutePath + "/lorien/" + "haizei.mp4"

        textureView = findViewById(R.id.texture_view)
        textureView.surfaceTextureListener = this

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
        // 从native获取视频size
        val videoWidth = mediaPlayer?.getMediaParams(MEDIA_PARAM_VIDEO_WIDTH)
        val videoHeight = mediaPlayer?.getMediaParams(MEDIA_PARAM_VIDEO_HEIGHT)

        if (videoWidth!! * videoHeight!! != 0L) {
            // 根据视频size，调整TextureView宽高比
            adjustAspectRatio(videoWidth, videoHeight)
        }

        val duration = mediaPlayer?.getMediaParams(MEDIA_PARAM_VIDEO_DURATION)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            seekBar.min = 0
        }
        seekBar.max = duration?.toInt()!!
    }

    private fun adjustAspectRatio(videoWidth: Long, videoHeight: Long) {
        val viewWidth = textureView.width
        val viewHeight = textureView.height
        // 视频纵横比
        val aspectRatio = videoHeight.toDouble() / videoWidth.toDouble()
        var newWidth = 0
        var newHeight = 0
        if (viewHeight > viewWidth * aspectRatio) {
            newWidth = viewWidth
            newHeight = (viewWidth * aspectRatio).toInt()
        } else {
            newHeight = viewHeight
            newWidth = (viewHeight / aspectRatio).toInt()
        }
        val xoff = (viewWidth - newWidth) / 2
        val yoff = (viewHeight - newHeight) / 2
        val txform = Matrix()
        textureView.getTransform(txform)
        val sx = newWidth.toFloat() / viewWidth.toFloat()
        val sy = newHeight.toFloat() / viewHeight.toFloat()
        Log.d(TAG, "adjustTextureAspectRatio video=${videoWidth}x${videoHeight} " +
                "view=${viewWidth}x${viewHeight} " +
                "newView=${newWidth}x${newHeight} " +
                "off=${xoff}x${yoff} scaleX=$sx scaleY=$sy")
        txform.setScale(sx, sy)
        txform.postTranslate(xoff.toFloat(), yoff.toFloat())
//        txform.postRotate(10F) // for fun
        textureView.setTransform(txform)
    }

    override fun onPlayerEvent(msgType: Int, msgValue: Float) {
        Log.d(TAG, "onPlayerEvent, msgType=$msgType, msgValue=$msgValue")
        runOnUiThread {
            when (msgType) {
                MSG_DECODER_READY -> {
                    // 播放器play后，native层准备好解码器后，解码loop之前，会回调这个方法
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

    override fun onSurfaceTextureAvailable(st: SurfaceTexture, width: Int, height: Int) {
        Log.d(TAG, "onSurfaceTextureAvailable width=$width, height=$height")
        mediaPlayer = MediaPlayer()
        mediaPlayer?.addEventCallback(this)
        val surface = Surface(st)
        // 初始化播放器
        mediaPlayer?.init(videoPath, VIDEO_RENDER_ANWINDOW, surface)

        // 开始播放
        mediaPlayer?.play()
    }

    override fun onSurfaceTextureSizeChanged(surface: SurfaceTexture, width: Int, height: Int) {
        // ignore
        Log.d(TAG, "onSurfaceTextureSizeChanged width=$width, height=$height")
    }

    override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean {
        Log.d(TAG, "onSurfaceTextureDestroyed")

        mediaPlayer?.stop()
        mediaPlayer?.unInit()

        return true
    }

    override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {
//        Log.d(TAG, "onSurfaceTextureUpdated")
    }
}