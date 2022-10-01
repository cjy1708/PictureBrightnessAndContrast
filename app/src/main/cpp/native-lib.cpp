#include <jni.h>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include "android/log.h"
#include<android/bitmap.h>


#define  TAG    "JNI_TAG"
// 定义info信息
#define LOGI(...)     __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

jstring chartoJstring(JNIEnv *env, const char *pat);

JavaVM *gJavaVM;//全局JavaVM 变量
jobject gJavaObj;//全局Jobject变量
JNIEnv * gEnv;	//全局的JNIEnv变量
jmethodID nativeCallback;//全局的方法ID
static int count = 0;
static void* native_thread_exec(void *arg)
{
    LOGE("nativeThreadExec");
    LOGE("The pthread id : %ld\n", pthread_self());
    //从全局的JavaVM中获取到环境变量
    JNIEnv *env;
    gJavaVM->AttachCurrentThread(&env,nullptr);
    //线程循环
    for(int i = 0 ; i < 5; i++)
    {
        usleep(20);
        //跨线程回调Java层函数
        env->CallVoidMethod(gJavaObj,nativeCallback,count++);
//        gEnv->CallVoidMethod(gJavaObj,nativeCallback,count++);
    }
    gJavaVM->DetachCurrentThread();
    LOGE("thread stoped");
    return ((void *)0); //注意这里的sigtrap 必须返回
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_luge_jnidemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_luge_jnidemo_MainActivity_helloJni(JNIEnv *env, jobject thiz, jstring str) {
    /***
     * 因为 Java 默认使用 Unicode 编码，而 C/C++ 默认使用 UTF 编码，所以在本地代码中操作字符串的时候，
     * 必须使用合适的 JNI 函数把 jstring 转换成 C 风格的字符串。 编码的问题 很是关键
     */
    char const *c_str = NULL;
    jboolean isCopy; // 返回JNI_TRUE表示原字符串的拷贝，返回JNI_FALSE表示返回原字符串的指针
    c_str = env->GetStringUTFChars(str, &isCopy);//从jstring指针中获取数据
    LOGE("isCopy %d\n", isCopy);
    if(c_str == NULL)//判断是否获取数据成功
    {
        return NULL;
    }
    LOGE("The String from Java : %s\n", c_str);
    env->ReleaseStringUTFChars(str, c_str);
    char buff[128] = "Hello Im from JNI!";
    //仔细思考这里的代码的问题 是否可以 不规范的也是有问题的 这里的意思要深入理解 也就是说 newStringUTF 是在jvm虚拟机
    //里面开辟的内存 如果这个时候内存不足 这里有问题的 我们应该怎么处理？DETECTED ERROR IN APPLICATION: input is not valid Modified UTF-8: illegal start byte 0x80，
//    return env->NewStringUTF(buff);

    for(int i = 0; i < 3000; i++){
        jclass jString = env->FindClass("java/lang/String");
        jcharArray charArray = env->NewCharArray(100);
        jstring jstr = env->NewStringUTF("hello world");
//        jstring strlocalRef = static_cast<jstring>(env->NewLocalRef(jString));
//        LOGE("%s",jstr);
//        env->ReleaseStringUTFChars(jstr,"utf-8");
    }

    return chartoJstring(env,buff);
}

jstring chartoJstring(JNIEnv *env, const char *pat) {
    jclass strClass = env->FindClass("java/lang/String");
    jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes = env->NewByteArray((jsize) strlen(pat));
    env->SetByteArrayRegion(bytes, 0, (jsize) strlen(pat), (jbyte *) pat);
    jstring encoding = env->NewStringUTF("utf-8");
    return (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_luge_jnidemo_MainActivity_helloJni2(JNIEnv *env, jobject thiz, jstring j_str) {
    jsize len = env->GetStringUTFLength(j_str); // 获取utf-8字符串的长度
    LOGE("str_len:%d\n",len);
    char buff[128] = "hello ";
    char* pBuff = buff + 6;
    // 将虚拟机平台中的字符串以utf-8编码拷入C缓冲区,该函数内部不会分配内存空间
    env->GetStringUTFRegion(j_str,0,len,pBuff); //底层没有开辟相关的ReleaseStringUTFRegion这样的函数
    return env->NewStringUTF(buff);
}


extern "C"
JNIEXPORT jintArray JNICALL
Java_com_luge_jnidemo_MainActivity_operateIntArray(JNIEnv *env, jobject thiz, jintArray intArray_in) {
/**********	解析从Java Native得到的jintArray数组 **********/
    jint * intArray;
    //操作方式一:
    // 如同 getUTFString 一样，会申请 native 内存
    intArray= env->GetIntArrayElements(intArray_in, NULL);
    if(intArray == NULL){
        return NULL;
    }
    //得到数组的长度
    const int  length = env->GetArrayLength(intArray_in);
    LOGE("The intArray length is : %d\n", length);
    for(int i = 0; i < length; i++)
    {
        LOGE("The intArray_in[%d} is %d\n", i,intArray[i]);
    }
    //操作方法二
    if(length > 0)
    {
        jint * buf_in;//直接定义一个jni型数组
        buf_in = (int *)malloc(sizeof(int) * length);
        //通过GetIntArrayRegion方法获取数组内容，此种方法不会申请内存
        env->GetIntArrayRegion(intArray_in, 0, length, buf_in);
        free(buf_in);
    }
    //对于操作方式一，使用完了一定不要忘了释放内存
    env->ReleaseIntArrayElements(intArray_in, intArray, 0);
    /**********从JNI返回jintArray数组给Java层**********/
    jintArray intArray_out;//返回给java
    const int len_out = 10;
    intArray_out = env->NewIntArray(len_out);//在Native层创建一个长度为10的int型数组
    jint buf_out[len_out] = {0};
    for(int j = 0; j < len_out; j++){
        buf_out[j]  = j * 2;
    }
    //使用SetIntArrayRegion来赋值
    env->SetIntArrayRegion(intArray_out, 0, len_out, buf_out);
    return intArray_out;

}


extern "C"
JNIEXPORT void JNICALL
Java_com_luge_jnidemo_MainActivity_nativeInit(JNIEnv *env, jobject object) {
    LOGE("Java_com_xxx_api_thread_NativeThread_nativeInit\n");

    gJavaObj = env->NewGlobalRef(object);//创建全局引用
    jclass clazz = env->GetObjectClass(object);
    nativeCallback = env->GetMethodID(clazz,"onNativeCallBack","(I)V");
    env->GetJavaVM(&gJavaVM);
//    gJavaObj = object;//保存object到全局gJavaObj中
//    gEnv = env;
//    jclass clazz = env->GetObjectClass(object);
//    nativeCallback = env->GetMethodID(clazz,"onNativeCallBack","(I)V");
    pthread_t id;
    //通过pthread库创建线程
    LOGE("create native thread\n");
    if(pthread_create(&id,nullptr,native_thread_exec,nullptr)!=0)
    {
        LOGE("native thread create fail");
        return;
    }
    for(int i = 0 ; i < 5; i++)
    {
        usleep(20);
        //跨线程回调Java层函数
      //  env->CallVoidMethod(gJavaObj,nativeCallback,count++);
       // gEnv->CallVoidMethod(gJavaObj,nativeCallback,count++);
    }
    LOGE("native thread creat success");
}




extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGE("JNI_OnLoad\n");
    JNIEnv* env = NULL;
    //获取JNI_VERSION版本
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        LOGE("checkversion error\n");
        return -1;
    }
    //操作方式一，通过SO加载时保存全局JavaVM
    gJavaVM = vm;
    //返回jni 的版本
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_luge_jnidemo_MainActivity_getStrings(JNIEnv *env, jobject thiz, jint count,
                                              jstring sample) {
    jobjectArray str_array = NULL;
    jclass cls_string = NULL;
    jmethodID mid_string_init;
    jobject obj_str = NULL;
    const char *c_str_sample = NULL;
    char buff[256];
    int i;

    // 保证至少可以创建3个局部引用（str_array，cls_string，obj_str）
    if (env->EnsureLocalCapacity(3) != JNI_OK) {
        return NULL;
    }

    c_str_sample = env->GetStringUTFChars(sample, NULL);
    if (c_str_sample == NULL) {
        return NULL;
    }

    cls_string = env->FindClass("java/lang/String");
    if (cls_string == NULL) {
        return NULL;
    }

    // 获取String的构造方法
    mid_string_init = env->GetMethodID(cls_string, "<init>", "()V");
    if (mid_string_init == NULL) {
        env->DeleteLocalRef(cls_string);
        return NULL;
    }
    obj_str = env->NewObject(cls_string, mid_string_init);
    if (obj_str == NULL) {
        env->DeleteLocalRef(cls_string);
        return NULL;
    }

    // 创建一个字符串数组
    str_array = env->NewObjectArray(count, cls_string, obj_str);
    if (str_array == NULL) {
        env->DeleteLocalRef(cls_string);
        env->DeleteLocalRef(obj_str);
        return NULL;
    }

    // 给数组中每个元素赋值
    for (i = 0; i < count; ++i) {
        memset(buff, 0, sizeof(buff));   // 初始一下缓冲区
        sprintf(buff, c_str_sample,i);
        jstring newStr = env->NewStringUTF(buff);
        env->SetObjectArrayElement(str_array, i, newStr);
    }

    // 释放模板字符串所占的内存
    env->ReleaseStringUTFChars(sample, c_str_sample);

    // 释放局部引用所占用的资源
    env->DeleteLocalRef(cls_string);
    env->DeleteLocalRef(obj_str);

    return str_array;
}



extern "C"
JNIEXPORT jintArray JNICALL
Java_com_luge_jnidemo_MainActivity_getImage(JNIEnv *env, jobject thiz, jintArray buffer, jint width,
                                            jint height) {

    jint* source = env->GetIntArrayElements(buffer, nullptr);
    int newsize = width * height;
    //亮度、对比度 这两个参数可以传进来
    float brightness = 0.2f, constrat = 0.2f;
    int bab = (int)(255 * brightness);

    //开始处理
    int a, r, g, b;
    //实际设置的对比度2
    int cab = (int)(constrat * 65536) + 1;
    //遍历所有的像素点
    int x = 0, y = 0;
    for(x = 0; x < width; x++){
        for(y = 0; y < height; y++){
            //获得每个像素点的颜色值
            int color = source[y * width + x];
            /*大家可以看android的源码中的实现
            a = Color.alpha(color);  a = color >>> 24; 但是c中不支持左移24位
            r = Color.red(color);
            g = Color.green(color);
            b = Color.blue(color);*/
            a = (color >> 24) & 0xFF;
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = color & 0xFF;
            //美白argb的值都变大
            //美黑argb的值都变小
            int rr = r - bab;
            int gr = g - bab;
            int br = b - bab;

            //边界检测
            r = rr > 255 ? 255 : (rr < 0 ? 0 : rr);
            g = gr > 255 ? 255 : (gr < 0 ? 0 : gr);
            b = br > 255 ? 255 : (br < 0 ? 0 : br);

            //~~对比度变化，255的一半来比较  策略：让比较亮的更加量，让比较暗的更加暗
            //int ri = r - 128;
            //int gi = g - 128;
            //int bi = b - 128;
            int ri = ((r - 128) * cab) >> 16 + 128;  //位移十六位 相当于对比度扩大
            int gi = ((g - 128) * cab) >> 16 + 128;
            int bi = ((b - 128) * cab) >> 16 + 128;

            //边界检测
            r = rr > 255 ? 255 : (rr < 0 ? 0 : rr);
            g = gr > 255 ? 255 : (gr < 0 ? 0 : gr);
            b = br > 255 ? 255 : (br < 0 ? 0 : br);

            //设置图像像素点的调整后的色值
            //result.setPixel(x, y,  Color.argb(a, r, g, b));//参照android的源码
            int newColor = (a << 24) | (r << 16) | (g << 8) | b;
            source[y * width + x] = newColor;   //设置到原先值
        }
    }
    //指针转成jint
    jintArray result = env->NewIntArray(newsize);
    env->SetIntArrayRegion(result , 0, newsize, source);
    //释放内存
    env->ReleaseIntArrayElements(buffer, source, 0);
    return result;
}