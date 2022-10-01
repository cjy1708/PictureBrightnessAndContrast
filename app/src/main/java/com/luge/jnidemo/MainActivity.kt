package com.luge.jnidemo

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Log
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import com.luge.jnidemo.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private val TAG = "NativeThread"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        println(helloJni("test java "))
        operateIntArray()
        nativeInit()
        getStrings(10000,"java !!!!")
        ndkImage(binding.newImage)
    }

    /**
     * A native method that is implemented by the 'jnidemo' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String


    external fun helloJni(str: String): String //Java的Native方法
    external fun helloJni2(str: String): String //Java的Native方法
    external fun operateIntArray(array: IntArray): IntArray // 一维数组入参和作为返回值
    external fun nativeInit() //Native方法
    external fun getStrings(count: Int, sample: String): Array<String>

    /***
     * 获取图片的亮度和饱和度
     */
    external fun getImage(buffer: IntArray, width: Int, height: Int): IntArray




    fun onNativeCallBack(count: Int) {
        Log.e(TAG, "onNativeCallBack : $count")
    }

    private fun operateIntArray() {
        val array_int = intArrayOf(10, 100)
        val array_out: IntArray = operateIntArray(array_int)
        for (i in array_out.indices) {
            Log.e("JNI_ARRAY", "operateIntArray : " + array_out[i])
        }
    }

    /***
     * NDK 处理图片
     */
    fun ndkImage(imageView: ImageView) {
        val bitmap = BitmapFactory.decodeResource(resources,R.drawable.meinv)

        val start = System.currentTimeMillis()
        val width = bitmap.width
        val height = bitmap.height
        val buffer = IntArray(width * height)
        bitmap.getPixels(buffer, 0, width, 1, 1, width - 1, height - 1)
        val resultArray: IntArray = getImage(buffer, width, height)
        val result = Bitmap.createBitmap(resultArray, width, height, Bitmap.Config.RGB_565)
        val end = System.currentTimeMillis()
        binding.sampleText.text = "ndk处理时间---" + (end - start) + "ms"
        imageView.setImageBitmap(result)
    }


    companion object {
        // Used to load the 'jnidemo' library on application startup.
        init {
            System.loadLibrary("jnidemo")
        }
    }
}