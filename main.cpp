#include <iostream>
#include <coroutine>
#include <thread>
#include <future>
#include <chrono>
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

Task<int, AsyncDispatcher> sleep_task() {
    debug("Enter sleep_task")
    using namespace std::chrono_literals;
    // 天若有情天亦老
    // 我为长者续一秒
    co_await 1s;

    auto now = std::chrono::system_clock::now();
    auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    // 年轻人还是要提高姿势水平
    // 不要总想搞个大新闻
    co_await 1s;
    now = std::chrono::system_clock::now();
    auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::cout << "diff time: " << end_time - start_time << std::endl;
    debug("End sleep_task")
    co_return 0;
}

int main() {
    auto task = sleep_task();
    task.execute();
//    Looper& looper = Looper::getMainLooper();
//    debug("This is Main Thread!")
//    auto task = simple_task();
//    task.enqueue([](int result) {
//        std::cout << "Get Result: " << result << std::endl;
//    });
//    debug("End!")
//    looper.loop();
}