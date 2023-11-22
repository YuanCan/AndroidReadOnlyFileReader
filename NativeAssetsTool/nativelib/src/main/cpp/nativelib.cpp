      
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

JNIEXPORT void *JNICALL OpenFile(const char* fileName){
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
    return (void *)asset;
}

JNIEXPORT void JNICALL CloseFile(void * ptrAddress){
    AAsset *asset = (AAsset*)ptrAddress;
    AAsset_close(asset);
}

JNIEXPORT int64_t JNICALL GetLength(void * ptrAddress){
    AAsset *asset = (AAsset*)ptrAddress;
    int64_t length = AAsset_getLength64(asset);
    return length;
}

JNIEXPORT int64_t JNICALL GetPosition(void * ptrAddress){
    AAsset *asset = (AAsset*)ptrAddress;
    int64_t position = AAsset_getLength64(asset) - AAsset_getRemainingLength64(asset);
    return position;
}

JNIEXPORT int64_t JNICALL Seek(void * ptrAddress,int64_t offset,int32_t whence){
    AAsset *asset = (AAsset*)ptrAddress;
    return AAsset_seek64(asset,offset,whence);
}

JNIEXPORT int32_t JNICALL Read(void * ptrAddress,void *buffer,int32_t count){
    AAsset *asset = (AAsset*)ptrAddress;
    try{
        int32_t readCount = AAsset_read(asset,buffer,count);
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
