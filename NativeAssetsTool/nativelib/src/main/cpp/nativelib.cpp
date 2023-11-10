      
#include <jni.h>
#include <string>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#ifdef __cplusplus
extern "C" {
#endif
JNIEnv *env = nullptr;
AAssetManager *nativeAsset = nullptr;
JNIEXPORT jstring JNICALL Java_com_droidhang_nativelib_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "C++ asset tool";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm,void *reserved){
    jint result = -1;
    if(jvm->GetEnv((void **)&env,JNI_VERSION_1_4) != JNI_OK){
        return -1;
    }

    __android_log_print(ANDROID_LOG_DEBUG,"native_asset_read","Init success");
    return JNI_VERSION_1_4;
}

JNIEXPORT long JNICALL OpenFile(const char* fileName){
    if(nativeAsset == nullptr){
        jclass unityPlayerClass = env->FindClass("com/unity3d/player/UnityPlayer");
        jfieldID jfieldId = env->GetStaticFieldID(unityPlayerClass,"currentActivity","Landroid/app/Activity;");
        jobject currentActivity = env->GetStaticObjectField(unityPlayerClass,jfieldId);
        jclass activityClass = env->GetObjectClass(currentActivity);
        jmethodID activity_class_getAssets = env->GetMethodID(activityClass, "getAssets", "()Landroid/content/res/AssetManager;");
        jobject asset_manager = env->CallObjectMethod(currentActivity, activity_class_getAssets); // activity.getAssets();
        nativeAsset = AAssetManager_fromJava(env,asset_manager);
    }
    AAsset *asset = AAssetManager_open(nativeAsset,fileName,AASSET_MODE_STREAMING);
    long address = (long)asset;
    return address;
}

JNIEXPORT void JNICALL CloseFile(long ptrAddress){
    AAsset *asset = (AAsset*)ptrAddress;
    AAsset_close(asset);
}

JNIEXPORT long JNICALL GetLength(long ptrAddress){
    AAsset *asset = (AAsset*)ptrAddress;
    long length = AAsset_getLength64(asset);
    return length;
}

JNIEXPORT long JNICALL GetPosition(long ptrAddress){
    AAsset *asset = (AAsset*)ptrAddress;
    long position = AAsset_getLength64(asset) - AAsset_getRemainingLength64(asset);
    return position;
}

JNIEXPORT long JNICALL Seek(long ptrAddress,long offset,int whence){
    AAsset *asset = (AAsset*)ptrAddress;
    return AAsset_seek64(asset,offset,whence);
}

JNIEXPORT int JNICALL Read(long ptrAddress,void *buffer,int count){
    AAsset *asset = (AAsset*)ptrAddress;
    try{
        int readCount = AAsset_read(asset,buffer,count);
        return readCount;
    }
    catch (...){
        __android_log_print(ANDROID_LOG_DEBUG,"native_asset_read","exception");
        return 0;
    }
}

#ifdef __cplusplus
}
#endif