#include "HelloWorldScene.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <android/log.h>
#include <jni.h>
#include <pthread.h>
#define  LOG_TAG    "myErr"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

USING_NS_CC;

pthread_mutex_t* mutex1 = NULL;
cv::Mat* HelloWorld::drawing_frame = NULL;
cv::Mat* HelloWorld::grayImage = NULL;

Scene* HelloWorld::createScene(cv::Mat* gray, cv::Mat* channel3Mat, pthread_mutex_t* mutex)
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    //get vuforia greyImage
    grayImage = gray;
    //get vuforia rgbImage
    drawing_frame = channel3Mat;
    //get mutex
    mutex1 = mutex;

    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.
    this->capture = Sprite::create("HelloWorld2.png");
    // this->capture->setAnchorPoint(Vec2(0.0f, 0.0f));
    this->capture->setPosition3D(Vec3(0, 0, -1100));
    this->capture->setCameraMask((unsigned short)CameraFlag::USER2);
    this->capture->setGlobalZOrder(-1);
    this->addChild(this->capture);

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);

    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    //auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    //sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    //this->addChild(sprite, 0);

    this->_spriteCamera = Camera::createPerspective(5.419f, (GLfloat)visibleSize.width / visibleSize.height, 1081.804810, 5000);

    this->_spriteCamera->setCameraFlag(CameraFlag::USER2);
    this->_spriteCamera->setPosition3D(Vec3(0, 0, 1081.804810));
    this->_spriteCamera->lookAt(Vec3(0, 0, 0), Vec3(0, 1, 0));
    this->addChild(_spriteCamera);

    this->scheduleUpdate();
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void HelloWorld::update(float dt)
{
        cv::Mat tempGrayMat;
        cv::Mat tempMat;
        pthread_mutex_lock(mutex1);
        tempMat = (*drawing_frame).clone();
        pthread_mutex_unlock(mutex1);
        //cv::flip(tempMat, tempMat, 1);
        cvtColor(tempMat, tempGrayMat, CV_RGB2GRAY);
        if(tempMat.data != NULL)
        {
            LOGE("Vuforia RGB get it there!");
            auto texture = new Texture2D;
            texture->initWithData(tempMat.data, tempMat.elemSize() * tempMat.cols * tempMat.rows, Texture2D::PixelFormat::RGB888,
                                  tempMat.cols, tempMat.rows,
                                  Size(tempMat.cols, tempMat.rows));
            LOGE("texture1 reference count: %d", texture->getReferenceCount());
            this->capture->setTexture(texture);
            texture->autorelease();
            LOGE("texture2 reference count: %d", texture->getReferenceCount());
        }
        else
        {
            LOGE("Vuforia RGB get NULL!");
        }
}
