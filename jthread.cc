#include <modcpp/base.h>
#include <condition_variable>
#include <thread>
#include <cassert>

void request_stop_example() {
    std::jthread th([](std::stop_token token, int args) {
        for (;;) {
            std::cout << "running:" << args <<  std::endl;
            if (token.stop_requested()) return;
            sleep(1);
        }
    }, 42);
    th.request_stop();
    while (1);
}

static constinit std::mutex mtx;
std::condition_variable_any cond;
void request_stop_on_condition_variable() {
    std::jthread th([](std::stop_token token) {
        for (;;) {
            std::unique_lock<std::mutex> lk(mtx);
            bool ret = cond.wait(lk, token, []() { return false; });
            if (ret) {
                std::cout << "condition holds" << std::endl;
            } else {
                std::cout << "should end" << std::endl;
                return;
            }
        }
    });
    th.request_stop();
    while (1);

}

void jthread_routine() {
    // request_stop_example();
    request_stop_on_condition_variable();
}
