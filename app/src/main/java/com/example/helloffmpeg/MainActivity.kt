package com.example.helloffmpeg

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.helloffmpeg.media.FFMediaPlayer

class MainActivity : AppCompatActivity() {

    private val requestPermissions = arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE)
    private val permissionRequestCode = 1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        findViewById<TextView>(R.id.text).text = FFMediaPlayer.getFFmpegVersion()
    }

    override fun onResume() {
        super.onResume()
        if (!hasPermissionGranted(requestPermissions)) {
            ActivityCompat.requestPermissions(this, requestPermissions, permissionRequestCode)
        }
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
}