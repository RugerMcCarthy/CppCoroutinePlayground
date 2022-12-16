#include <coroutine>
#include <iostream>
struct Generator {
    struct promise_type {
        int value;
        bool is_ready = false;
        void unexpected_handler() {}
        void unhandled_exception() {}
        std::suspend_always initial_suspend() {
            return {};
        }
        std::suspend_always final_suspend() noexcept{
            return {};
        }
        std::suspend_always await_transform(int value) {
            this->value = value;
            this->is_ready = true;
            return {};
        }

        Generator get_return_object() {
            return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        void return_void() {}

    };

    int value() {
        if (hasNext()) {
            return handler.promise().value;
        }
        throw "Out Of Limit";
    }

    void next() {
        if (hasNext()){
            handler.promise().is_ready = false;
        } else {
            throw "Out Of Limit";
        }
    }
    bool hasNext() {
        if (handler.done()) {
            return false;
        }
        if (!handler.promise().is_ready) {
            handler.resume();
        }
        if (handler.done()) {
            return false;
        } else {
            return true;
        }
    }

    struct Iterator {
        int value = 0;
        Iterator(Generator* generator): generator(generator) {};
        bool operator!=(Iterator it) {
            return generator->hasNext();
        }
        Iterator& operator++() {
            generator->next();
        }
        int operator*() {
            return generator->value();
        }
        Generator* generator = nullptr;
    };
    Iterator begin() {
        return Iterator(this);
    }
    Iterator end() {
        return Iterator(this);
    }
    std::coroutine_handle<promise_type> handler;
};

Generator sequence() {
    int i = 0;
    while (i < 5) {
        i++;
        co_await i;
    }
}

int main() {
    Generator gen = sequence();
    for (auto num: gen) {
        std::cout << num << std::endl;
    }
}