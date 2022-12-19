#include <iostream>
#include <coroutine>
#include <thread>
#include <future>

#include "include/Dispatcher.hpp"
#include "include/Looper.hpp"

#include "include/TimeUntil.hpp"
#ifndef TASK
    #include "Task.cpp"
#endif

Task<int, MainLooperDispatcher> simple_task2() {
    debug("Enter simple_task2")
    // sleep 1 秒
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    co_return 2;
}

Task<int, MainLooperDispatcher> simple_task3() {
    debug("Enter simple_task3")
    // sleep 2 秒
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);

    co_return 3;
}

Task<int, AsyncDispatcher> simple_task() {
    debug("Enter simple_task")
    auto result2 = co_await simple_task2();
    auto result3 = co_await simple_task3();
    debug("simple_task continue")
    co_return 1 + result2 + result3;
}


int main() {
//    Looper& looper = Looper::getMainLooper();
//    debug("This is Main Thread!")
//    auto task = simple_task();
//    task.enqueue([](int result) {
//        std::cout << "Get Result: " << result << std::endl;
//    });
//    debug("End!")
//    looper.loop();
}