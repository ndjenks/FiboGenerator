#include <coroutine>
#include <cstdlib>
#include "scoppedLogger.h"

constexpr size_t ARRAY_SIZE = 10;
struct Fibogenerator {
    // Compiler expects an object with this exact name
    struct promise_type {
        Fibogenerator get_return_object(){ return {Fibogenerator(std::coroutine_handle<promise_type>::from_promise(*this))};};
        auto initial_suspend() {return std::suspend_never{};};
        auto final_suspend() noexcept {return std::suspend_always{};};
        void unhandled_exception() {std::abort();}
        void return_value(int value) {returned_value = value;};
        auto yield_value(int value) {
            if (value > 1)
                fibonacci_sequence[value] = fibonacci_sequence[value - 2] + fibonacci_sequence[value - 1];
            return std::suspend_always{};
        };
        std::array<int,ARRAY_SIZE> fibonacci_sequence {0 , 1};
        int returned_value;
    };

    // Handle to control the promise_type from outside
    // f.ex resume or destroy coroutine or check if task is done
    std::coroutine_handle<promise_type> handle;

    ~Fibogenerator() {
        if (handle)
            handle.destroy();
    }
    auto operator=(Fibogenerator&&) = delete;

    // helper functions to get infos from promise_type
    int get_returned_value() {return handle.promise().returned_value;}
    std::array<int, ARRAY_SIZE> get_fibonacci_sequence() {return handle.promise().fibonacci_sequence;}
    void resume() {
        handle.resume();
    }
    bool done() {
        return handle.done();
    }
};

Fibogenerator f() {
    int no_of_fibonacci = ARRAY_SIZE;
    for (int i = 2; i < no_of_fibonacci; i++ ) {
        co_yield i;
    }
    co_return 0;
}

int main() {
    auto task = f();
    while (!task.done())
        task.resume();
    for (int fibonacci_number : task.get_fibonacci_sequence())
        LOG(fibonacci_number);
    LOG(task.get_returned_value());
    return 0;
}