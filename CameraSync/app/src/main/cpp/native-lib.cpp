#include <jni.h>
#include <string>


extern "C"
JNIEXPORT jstring JNICALL
Java_com_piotrmeller_camerasync_activity_MainActivity_stringFromJNI(JNIEnv *env, jobject instance) {

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}