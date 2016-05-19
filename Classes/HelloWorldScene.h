#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene(cv::Mat*, cv::Mat*, pthread_mutex_t*);

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

public:
    void update(float dt);
    static cv::Mat* drawing_frame;
    static cv::Mat* grayImage;
    cocos2d::Sprite* capture;
    cocos2d::Camera* _spriteCamera;
};

#endif // __HELLOWORLD_SCENE_H__
