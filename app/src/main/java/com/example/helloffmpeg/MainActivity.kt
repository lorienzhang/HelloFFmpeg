package com.example.helloffmpeg

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.helloffmpeg.media.MediaPlayer
import java.io.File
import java.io.FileOutputStream
import java.lang.Exception

private const val TAG = "lorien"

class MainActivity : AppCompatActivity() {

    private val requestPermissions = arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE)
    private val permissionRequestCode = 1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        findViewById<TextView>(R.id.text).text = MediaPlayer.getFFmpegVersion()
    }

    override fun onResume() {
        super.onResume()
        if (!hasPermissionGranted(requestPermissions)) {
            ActivityCompat.requestPermissions(this, requestPermissions, permissionRequestCode)
        }

        val root = getExternalFilesDir(null)
        copyAssetsDirToSDCard(this, "lorien", root!!.absolutePath)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        if (requestCode == permissionRequestCode) {
            if (!hasPermissionGranted(requestPermissions)) {
                Toast.makeText(this,
                    "we need WRITE_EXTERNAL_STORAGE permission",
                    Toast.LENGTH_SHORT).show()
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        }
    }

    private fun hasPermissionGranted(permissions: Array<String>): Boolean {
        for (per in permissions) {
            if (ActivityCompat.checkSelfPermission(this, per)
                != PackageManager.PERMISSION_GRANTED) {
                return false
            }
        }
        return true
    }

    private fun copyAssetsDirToSDCard(context: Context, assetsDir: String, sdCardPath: String) {
        Log.d(TAG, "copyAssetsDirToSDCard() called with: assetsFile=$assetsDir, sdCardPath=$sdCardPath")
        try {
            val list = context.assets.list(assetsDir)
            if (list?.size == 0) {
                val inputStream = context.assets.open(assetsDir)
                val byteArray = ByteArray(1024)
                var bt = 0
                val file = File(sdCardPath
                        + File.separator
                        + assetsDir.substring(assetsDir.lastIndexOf('/')))
                if (!file.exists()) {
                    file.createNewFile()
                } else {
                    return
                }
                val fos = FileOutputStream(file)

                while ((inputStream.read(byteArray).also { bt = it }) != -1) {
                    fos.write(byteArray, 0, bt)
                }
                fos.flush()
                inputStream.close()
                fos.close()
            } else {
                var subDir = assetsDir
                if (assetsDir.contains("/")) {
                    subDir = assetsDir.substring(assetsDir.lastIndexOf('/') + 1)
                }
                val sdCardDir = sdCardPath + File.separator + subDir
                val sdFile = File(sdCardDir)
                if (!sdFile.exists()) {
                    sdFile.mkdirs()
                }
                if (list != null) {
                    for (file in list) {
                        copyAssetsDirToSDCard(context, assetsDir + File.separator + file, sdCardDir)
                    }
                }
            }
        } catch (e: Exception) {
            Log.d(TAG, "copyAssetsDirToSDCard, e=$e")
        }
    }

    fun nativeRenderPlayer(view: View) {
        startActivity(Intent(this, NativeRenderActivity::class.java))
    }

    fun nativeRenderTexturePlayer(view: View) {
        startActivity(Intent(this, NativeRenderTextureActivity::class.java))
    }
}