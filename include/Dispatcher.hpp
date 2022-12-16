//
// Created by Ruger on 2022/12/16.
//
#include "Looper.hpp"

#ifndef COROUTINE_PLAYGROUND_DISPATCHER_H
#define COROUTINE_PLAYGROUND_DISPATCHER_H

class BaseDispatcher {
public:
    virtual void execute(std::function<void()>&& func) = 0;
};

class AsyncDispatcher: public BaseDispatcher {
public:
    void execute(std::function<void()>&& func) override {
        std::thread(func).detach();
    }
};

class MainLooperDispatcher: public BaseDispatcher {
public:
    void execute(std::function<void()>&& func) override {
        mainLooper.post(func);
    }
private:
    Looper& mainLooper = Looper::getMainLooper();
};
#endif //COROUTINE_PLAYGROUND_DISPATCHER_H
