#ifndef COROUTINE_PLAYGROUND_LOOPER_HPP
#define COROUTINE_PLAYGROUND_LOOPER_HPP

#include <functional>
#include <queue>
#include <mutex>
#include <vector>
class Message {
public:
    Message(std::function<void()> action, long long scheduled_time):action_(action), scheduled_time(scheduled_time) {}
    Message(const Message& msg) {
        action_ = msg.action_;
        scheduled_time = msg.scheduled_time;
    }
    void operator()() {
        action_();
    }
    long long get_scheduled_time() {
        return scheduled_time;
    }
private:
    std::function<void()> action_;
    long long scheduled_time = 0;
};

struct MessageComparator {
    bool operator()(std::shared_ptr<Message> left, std::shared_ptr<Message> right) {
        return left.get()->get_scheduled_time() > right.get()->get_scheduled_time();
    }
};

class MessageQueue {
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    std::priority_queue<std::shared_ptr<Message>, std::vector<std::shared_ptr<Message>>, MessageComparator> queue;
public:
    void push(Message&& message) {
        std::shared_ptr<Message> msg(new Message(message));
        std::unique_lock<std::mutex> this_lock(_mutex);
        queue.push(msg);
        this_lock.unlock();
        _cv.notify_one();
    }
    std::shared_ptr<Message> poll() {
        std::unique_lock<std::mutex> this_lock(_mutex);
        while (queue.empty()) {
            _cv.wait(this_lock);
        }
        std::shared_ptr<Message> msg = queue.top();
        auto now = std::chrono::system_clock::now();
        auto current = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        auto scheduled_time = msg.get()->get_scheduled_time();
        auto time_diff = scheduled_time - current;
        if (time_diff > 0) {
            auto status = _cv.wait_for(this_lock, std::chrono::milliseconds(time_diff));
        }
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
    void post(std::function<void()>&& func) {
        postDelay(std::move(func), 0);
    }
    void postDelay(std::function<void()> func, long long delay) {
        auto now = std::chrono::system_clock::now();
        auto scheduled_time = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        queue.push(Message(func, scheduled_time + delay));
    }
    static Looper& getMainLooper() {
        static Looper looper;
        return looper;
    }
};
#endif //COROUTINE_PLAYGROUND_LOOPER_HPP