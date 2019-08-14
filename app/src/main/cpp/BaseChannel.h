//
// Created by lixiaoxu on 2019-08-14.
//

#ifndef HAWKEYEPLAYER_BASECHANNEL_H
#define HAWKEYEPLAYER_BASECHANNEL_H

/**
 * VideoChannel 和AudilChannel的父类
 */

class BaseChannel {
public:
    BaseChannel() {

    }

    //虚函数析构函数，需要子类实现的
    //不然释放的时候会有问题
    virtual ~BaseChannel() {

    }
};

#endif //HAWKEYEPLAYER_BASECHANNEL_H
