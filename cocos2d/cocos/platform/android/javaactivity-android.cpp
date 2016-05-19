/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include "CCApplication-android.h"
#include "CCGLViewImpl-android.h"
#include "base/CCDirector.h"
#include "base/CCEventCustom.h"
#include "base/CCEventType.h"
#include "base/CCEventDispatcher.h"
#include "renderer/CCGLProgramCache.h"
#include "renderer/CCTextureCache.h"
#include "renderer/ccGLStateCache.h"
#include "2d/CCDrawingPrimitives.h"
#include "platform/android/jni/JniHelper.h"
#include <android/log.h>
#include <jni.h>

//Added by winter on 2016-3-28
#include <Vuforia/Vuforia.h>
#include <Vuforia/CameraDevice.h>
#include <Vuforia/UpdateCallback.h>
#include <Vuforia/State.h>
#include <Vuforia/Frame.h>
#include <Vuforia/Image.h>
#include <opencv2/opencv.hpp>
#include <pthread.h>

#define  LOG_TAG    "myErr"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

//Added by winter on 2016-3-28
static cv::Mat channel2Mat = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC2);
static cv::Mat channel3Mat = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC3);
static cv::Mat grayMat = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC1);
static jobject activityObj = 0;
pthread_mutex_t mutex;

void cocos_android_app_init(JNIEnv* env) __attribute__((weak));

using namespace cocos2d;

extern "C"
{

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JniHelper::setJavaVM(vm);

    cocos_android_app_init(JniHelper::getEnv());

    return JNI_VERSION_1_4;
}


//Added by winter on 2016-3-28
    class Vuforia_updateCallback : public Vuforia::UpdateCallback
    {
        virtual void Vuforia_onUpdate(Vuforia::State& state)
        {
            //LOGE("Vuforia_onUpdateNative!");
            Vuforia::Image *imageRGB888 = NULL;
            Vuforia::Image* imageGRAY = NULL;
            Vuforia::Frame frame = state.getFrame();

            for (int i = 0; i < frame.getNumImages(); ++i) {
                const Vuforia::Image *image = frame.getImage(i);
                if (image->getFormat() == Vuforia::RGB888) {
                    imageRGB888 = (Vuforia::Image*)image;

                    break;
                }
                else if(image->getFormat() == Vuforia::GRAYSCALE){
                    imageGRAY = (Vuforia::Image*)image;
                    break;
                }
            }

            if (imageRGB888) {
                JNIEnv* env = 0;
                if ((JniHelper::getJavaVM() != 0) && (activityObj != 0)
                    &&(JniHelper::getJavaVM()->GetEnv((void**)&env, JNI_VERSION_1_4) == JNI_OK)
                    ) {

                    // JNIEnv* env = JniHelper::getEnv();
                    const short* pixels = (const short*) imageRGB888->getPixels();
                    int width = imageRGB888->getWidth();
                    int height = imageRGB888->getHeight();
                    int numPixels = width * height;
                    jbyteArray pixelArray = env->NewByteArray(numPixels * 3);
                    env->SetByteArrayRegion(pixelArray, 0, numPixels * 3, (const jbyte*) pixels);

                    cv::Mat temp3Channel = cv::Mat(imageRGB888->getHeight(), imageRGB888->getWidth(), CV_8UC3, (unsigned char *)imageRGB888->getPixels());
            pthread_mutex_lock(&mutex);
                    channel3Mat = temp3Channel.clone();
            pthread_mutex_unlock(&mutex);
                    // imageBuffer = (char*)env->NewGlobalRef(pixelArray);
                    jclass javaClass = env->GetObjectClass(activityObj);
                    env->DeleteLocalRef(pixelArray);
                     //if (channel3Mat.data == NULL)
                       //  LOGE("channel2Mat is NULL");
                     //else
                       //  LOGE("channel2Mat get it!");
                    // JniHelper::getJavaVM()->DetachCurrentThread();
                }
            }
                else if (imageGRAY) {
                JNIEnv* env = 0;
                // JniHelper::getJavaVM()->AttachCurrentThread((void**)&env, NULL);

                if ((JniHelper::getJavaVM() != 0) && (activityObj != 0)
                    &&(JniHelper::getJavaVM()->GetEnv((void**)&env, JNI_VERSION_1_4) == JNI_OK)
                    ) {

                    // JNIEnv* env = JniHelper::getEnv();
                    const short* pixels = (const short*) imageGRAY->getPixels();
                    int width = imageGRAY->getWidth();
                    int height = imageGRAY->getHeight();
                    int numPixels = width * height;


                    jbyteArray pixelArray = env->NewByteArray(numPixels * 1);
                    env->SetByteArrayRegion(pixelArray, 0, numPixels * 1, (const jbyte*) pixels);

                    cv::Mat gray_frame = cv::Mat(480, 640, CV_8UC1, (unsigned char*)pixels, 0);
                    // cv::Mat mat = cv::Mat(imageRGB888->getHeight(), imageRGB888->getWidth(), CV_8UC1, (unsigned char *)imageRGB888->getPixels());
                    cv::Mat tempGray = cv::Mat(imageGRAY->getHeight(), imageGRAY->getWidth(), CV_8UC1, (unsigned char *)imageGRAY->getPixels());
            pthread_mutex_lock(&mutex);
                    // cv::cvtColor(temp3Channel, grayMat, CV_RGB2GRAY);
                    grayMat = gray_frame;
                    // imageBuffer = (char *)env->GetByteArrayElements(pixelArray, 0);
            pthread_mutex_unlock(&mutex);
                    jclass javaClass = env->GetObjectClass(activityObj);
                    env->DeleteLocalRef(pixelArray);
                }
            }
        }
    };

    Vuforia_updateCallback updateCallback;

    JNIEXPORT void JNICALL
    Java_com_vuforia_samples_SampleApplication_SampleApplicationSession_onVuforiaInitializedNative(JNIEnv * env, jobject)
    {
        LOGE("onVuforiaInitalizeNative!");

        // Register the update callback where we handle the data set swap:
        Vuforia::registerCallback(&updateCallback);

        // Comment in to enable tracking of up to 2 targets simultaneously and
        // split the work over multiple frames:
        // Vuforia::setHint(Vuforia::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
    }


JNIEXPORT void JNICALL
Java_org_cocos2dx_lib_Cocos2dxActivity_initApplicationNative(JNIEnv* env, jobject obj)
    {
        activityObj = env->NewGlobalRef(obj);
    }

JNIEXPORT void JNICALL
    Java_com_vuforia_samples_SampleApplication_SampleApplicationSession_startNativeCamera(JNIEnv *, jobject, jint camera)
    {
        LOGE("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_startCamera");
        // Start the camera:
        if (!Vuforia::CameraDevice::getInstance().start())
            return;
        Vuforia::setFrameFormat(Vuforia::RGB888, true);
    }

JNIEXPORT void JNICALL
     Java_com_vuforia_samples_SampleApplication_SampleApplicationSession_stopNativeCamera(JNIEnv *, jobject)
     {
         LOGE("Java_com_vuforia_samples_ImageTargets_ImageTargets_stopCamera");

         Vuforia::CameraDevice::getInstance().stop();
         Vuforia::CameraDevice::getInstance().deinit();
     }

JNIEXPORT void Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeInit(JNIEnv*  env, jobject thiz, jint w, jint h)
{
    auto director = cocos2d::Director::getInstance();
    auto glview = director->getOpenGLView();
    if (!glview)
    {
        glview = cocos2d::GLViewImpl::create("Android app");
        glview->setFrameSize(w, h);
        director->setOpenGLView(glview);

        cocos2d::Application::getInstance()->run(&grayMat, &channel3Mat, &mutex);
    }
    else
    {
        cocos2d::GL::invalidateStateCache();
        cocos2d::GLProgramCache::getInstance()->reloadDefaultGLPrograms();
        cocos2d::DrawPrimitives::init();
        cocos2d::VolatileTextureMgr::reloadAllTextures();

        cocos2d::EventCustom recreatedEvent(EVENT_RENDERER_RECREATED);
        director->getEventDispatcher()->dispatchEvent(&recreatedEvent);
        director->setGLDefaultValues();
    }
}

JNIEXPORT jintArray Java_org_cocos2dx_lib_Cocos2dxActivity_getGLContextAttrs(JNIEnv*  env, jobject thiz)
{
    cocos2d::Application::getInstance()->initGLContextAttrs(); 
    GLContextAttrs _glContextAttrs = GLView::getGLContextAttrs();
    
    int tmp[6] = {_glContextAttrs.redBits, _glContextAttrs.greenBits, _glContextAttrs.blueBits,
                           _glContextAttrs.alphaBits, _glContextAttrs.depthBits, _glContextAttrs.stencilBits};


    jintArray glContextAttrsJava = env->NewIntArray(6);
        env->SetIntArrayRegion(glContextAttrsJava, 0, 6, tmp); 
    
    return glContextAttrsJava;
}

JNIEXPORT void Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnSurfaceChanged(JNIEnv*  env, jobject thiz, jint w, jint h)
{
    cocos2d::Application::getInstance()->applicationScreenSizeChanged(w, h);
}

}

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

