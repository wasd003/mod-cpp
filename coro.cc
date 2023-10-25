#include <concepts>
#include <coroutine>
#include <exception>
#include <iostream>

struct future {

    struct promise_type {
        int value;
        /**
         * NOTE: this function only be executed when constructing
         * frame of coroutine
         */
        future get_return_object() {
            return {
                .coro_handle =
                    std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        /**
         * NOTE: whether back to caller after coroutine initialization
         */
        auto initial_suspend() noexcept { return std::suspend_always {}; }
        /**
         * NOTE: whether back to caller before coroutine destroy
         */
        auto final_suspend() noexcept { return std::suspend_never {}; }
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> coro_handle;

    operator std::coroutine_handle<promise_type>() {
        return coro_handle;
    }

    operator std::coroutine_handle<>() {
        return coro_handle;
    }
};

template<typename PromiseType>
struct current_promise {

    bool await_ready() {
        return false;
    }

    bool await_suspend(std::coroutine_handle<PromiseType> handle) {
        promise_ptr = &handle.promise();
        return false;
    }

    PromiseType* await_resume() {
        return promise_ptr;
    }

    private:
    PromiseType* promise_ptr;
};

future counter() {
    /* 
     * init procedure:
     * 1. get current promise
     * 2. return coro_handle to caller
     **/
    auto* promise = co_await current_promise<future::promise_type> {};

    int i = 0;
    for (;;) {
        promise->value = i ++ ;
        co_await std::suspend_always {};
    }
}

void coro_main() {

    std::coroutine_handle<future::promise_type> counter_handle = counter();
    auto &promise = counter_handle.promise();
    for (int i = 0; i < 3; i ++ ) {
        counter_handle.resume();
        std::cout << promise.value << std::endl;
    }
}
