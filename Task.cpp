//
// Created by Ruger on 2022/12/15.
//
#include <coroutine>
#include <functional>
#include <optional>
#include <mutex>
#include <list>
#include <concepts>
#include "include/Dispatcher.hpp"
#ifndef TASK
#define TASK


template<typename T>
struct Result {
    explicit Result() = default;

    explicit Result(T &&value) : _value(value) {}

    explicit Result(std::exception_ptr &&exception_ptr) : _exception_ptr(exception_ptr) {}

    T get_or_throw() {
        if (_exception_ptr) {
            std::rethrow_exception(_exception_ptr);
        }
        return _value;
    }
private:
    T _value{};
    std::exception_ptr _exception_ptr;
};

template<typename R, typename DISPATCHER>
requires std::derived_from<DISPATCHER, BaseDispatcher>
class Task;
// TaskAwaiter
template<typename R, typename DISPATCHER>
class TaskAwaiter {
public:
    // DISPATCHER为子协程调度器类型，dispatcher为当前协程调度器，类型可能不同。
    explicit TaskAwaiter(Task<R, DISPATCHER>&& task, BaseDispatcher* dispatcher):task_(task), dispatcher_(dispatcher){};

    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<> handle){
        task_.complete([handle, this](){
            dispatcher_->execute([handle]() {
                handle.resume();
            });
        });
    }

    R await_resume() {
        return task_.execute();
    }
private:
    Task<R, DISPATCHER> task_;
    BaseDispatcher* dispatcher_ = nullptr;
};

// InitialAwaiter
class InitialAwaiter {
public:
    InitialAwaiter(BaseDispatcher* dispatcher): dispatcher_(dispatcher){};

    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<> handle){
        dispatcher_->execute([handle](){
            handle.resume();
        });
    }

    void await_resume() {}
private:
    BaseDispatcher* dispatcher_ = nullptr;
};

template<typename R, typename DISPATCHER>
requires std::derived_from<DISPATCHER, BaseDispatcher>
class Task {
public:
    struct promise_type {
        InitialAwaiter initial_suspend() {
            return InitialAwaiter(&dispatcher);
        }
        std::suspend_always final_suspend() noexcept{
            return {};
        }
        void unhandled_exception() {
            std::unique_lock lock(mutex);
            result = Result<R>(std::current_exception());
            lock.unlock();
            conv.notify_all();
            notify_callbacks();
        }
        void return_value(R value) {
            std::unique_lock lock(mutex);
            result = Result<R>(std::move(value));
            lock.unlock();
            conv.notify_all();
            notify_callbacks();
        }

        template<typename __R, typename __DISPATCHER>
        TaskAwaiter<__R, __DISPATCHER> await_transform(Task<__R, __DISPATCHER>&& task) {
            return TaskAwaiter<__R, __DISPATCHER>(std::move(task), &dispatcher);
        }

        Task<R, DISPATCHER> get_return_object() {
            return Task(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        R get_result() {
            std::unique_lock lock(mutex);
            if (!result.has_value()) {
                conv.wait(lock);
            }
            return result.value().get_or_throw();
        }
        void on_completed(std::function<void(Result<R>)> func) {
            std::unique_lock lock(mutex);
            if (result.has_value()) {
                auto value = result.value();
                lock.unlock();
                func(value);
            } else {
                callbacks.push_back(func);
            }
        }
        private:
            std::optional<Result<R>> result;
            std::mutex mutex;
            std::condition_variable conv;
            std::list<std::function<void(Result<R>)>> callbacks;
            DISPATCHER dispatcher;
            void notify_callbacks() {
                auto value = result.value();
                for (auto &callback : callbacks) {
                    callback(value);
                }
                // 调用完成，清空回调
                callbacks.clear();
            }
    };

    Task(std::coroutine_handle<promise_type> handle): _handle(handle) {};

    R execute() {
        return _handle.promise().get_result();
    }


    void enqueue(std::function<void(R)> func) {
        _handle.promise().on_completed([func](Result<R> result) {
            func(result.get_or_throw());
        });
    }

    void complete(std::function<void()>&& func) {
        _handle.promise().on_completed([func](Result<R> result) {
            func();
        });
    }
private:
    std::coroutine_handle<promise_type> _handle;
};

#endif