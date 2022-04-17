package com.example.helloffmpeg.media

import android.content.Context
import android.util.AttributeSet
import android.util.Log
import android.view.SurfaceView

const val TAG = "lorien"

class SizeAdapterSurfaceView(context: Context, attrs: AttributeSet?) : SurfaceView(context, attrs) {
    constructor(context: Context) : this(context, null)

    private var mAspectWidth = 0
    private var mAdaptHeight = 0

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)

        val width = MeasureSpec.getSize(widthMeasureSpec)
        val height = MeasureSpec.getSize(heightMeasureSpec)

        if (mAspectWidth == 0 || mAdaptHeight == 0) {
            setMeasuredDimension(width, height)
        } else {
            if (width < height * mAspectWidth / mAdaptHeight) {
                setMeasuredDimension(width, width * mAdaptHeight / mAspectWidth)
            } else {
                setMeasuredDimension(height * mAspectWidth / mAdaptHeight, height)
            }
        }

        Log.d(TAG, "SurfaceView onMeasure, width=${measuredWidth}, height=${measuredHeight}")
    }

    fun setAspectRatio(width: Int, height: Int) {
        mAspectWidth = width
        mAdaptHeight = height
        requestLayout()
    }
}