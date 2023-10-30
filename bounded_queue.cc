#include <modcpp/base.h>
#include <condition_variable>
#include <queue>
#include <thread>


template<typename T, auto Capacity>
class bounded_queue {
protected:
    std::deque<T> queue;
    std::mutex mtx;
    std::condition_variable producer_cond, consumer_cond;

public:
    virtual void push(const T& data) = 0;

    virtual T pop() = 0;
};

template<typename T, auto Capacity>
class sleep_bounded_queue : public bounded_queue<T, Capacity> {
public:
    void push(const T& data) override {
        std::unique_lock<std::mutex> lk(this->mtx);
        this->producer_cond.wait(lk, [this]() {
            return this->queue.size() < Capacity;
        });
        this->queue.push_back(data);
        this->consumer_cond.notify_one();
    }

    T pop() override {
        std::unique_lock<std::mutex> lk(this->mtx);
        this->consumer_cond.wait(lk, [this]() {
            return !this->queue.empty();
        });
        auto ans = this->queue.front(); this->queue.pop_front();
        this->producer_cond.notify_one();
        return ans;
    }
};

template<typename T, auto Capacity>
class polling_bounded_queue : public bounded_queue<T, Capacity> {
    // see lockless_queue
};

void bounded_queue_routine() {
    sleep_bounded_queue<int, 10> sleep_queue;
    constexpr static int thread_nr = 12;
    std::vector<std::thread> producer_list, consumer_list;
    for (int i = 0; i < thread_nr; i ++ ) {
        consumer_list.emplace_back([&sleep_queue]() {
            for (;;) {
                auto val = sleep_queue.pop();
                std::cout << val << std::endl;
            }
        });
    }

    for (int i = 0; i < thread_nr; i ++ ) {
        producer_list.emplace_back([&sleep_queue]() {
            for (;;) {
                constexpr static int val = 42;
                sleep_queue.push(42);
            }
        });
    }
    for (int i = 0; i < thread_nr; i ++ ) {
        producer_list[i].join();
        consumer_list[i].join();
    }
}
