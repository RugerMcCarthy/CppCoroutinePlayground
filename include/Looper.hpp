#ifndef COROUTINE_PLAYGROUND_LOOPER_HPP
#define COROUTINE_PLAYGROUND_LOOPER_HPP

#include <functional>
#include <queue>
#include <mutex>
class Message {
public:
    Message(std::function<void()> action):action_(action) {}
    Message(const Message& msg) {
        action_ = msg.action_;
    }
    void operator()() {
        action_();
    }
private:
    std::function<void()> action_;
};
class MessageQueue {
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<std::shared_ptr<Message>> queue;
public:
    void push(Message message) {
        std::shared_ptr<Message> msg(new Message(message));
        std::unique_lock<std::mutex> this_lock(_mutex);
        queue.push(msg);
        this_lock.unlock();
        _cv.notify_one();
    }
    std::shared_ptr<Message> poll() {
        std::unique_lock<std::mutex> this_lock(_mutex);
        while (!queue.size()) {
            _cv.wait(this_lock);
        }
        std::shared_ptr<Message> msg = queue.front();
        queue.pop();
        return msg;
    }

};
class Looper {
private:
    MessageQueue queue;
public:
    Looper() = default;
    void loop() {
        while (true) {
            std::shared_ptr<Message> msg = queue.poll();
            (*msg.get())();
        }
    }
    void post(Message message) {
        queue.push(message);
    }
    static Looper& getMainLooper() {
        static Looper looper;
        return looper;
    }
};
#endif //COROUTINE_PLAYGROUND_LOOPER_HPP