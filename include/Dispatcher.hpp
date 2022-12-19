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
        mainLooper.post(std::move(func));
    }
private:
    Looper& mainLooper = Looper::getMainLooper();
};

class LooperScheduler {
public:
    LooperScheduler() {
        work_thread.detach();
    }
    void run_loop() {
        looper.loop();
    }
    void post(std::function<void()> func,  long long delay = 0) {
        looper.postDelay(std::move(func), delay);
    }
private:
    std::thread work_thread = std::thread(&LooperScheduler::run_loop, this);
    Looper looper;
};
#endif //COROUTINE_PLAYGROUND_DISPATCHER_H
