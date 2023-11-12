#include <modcpp/base.h>
#include <cassert>
#include <queue>
#include <memory>
#include <thread>
#include <atomic>
#include <optional>

using namespace std;

template<typename T, auto QueueCapacity = 4>
class lockless_queue {
private:
    T data[QueueCapacity];
    /**
     *
     * @push_idx: the first index can be pushed
     * @pop_idx: the first index can be poped
     */
    std::atomic<int> push_idx, shadow_push_idx;
    std::atomic<int> pop_idx, shadow_pop_idx;

    bool can_push(int current_push_idx) {
        return (current_push_idx + 1) % QueueCapacity != shadow_pop_idx.load();
    }

    bool can_pop(int current_pop_idx) {
        return current_pop_idx != shadow_push_idx.load();
    }

public:
    void push(const T& val) {
        int current_push_idx = push_idx.load();
        bool flag;
        while ((flag = can_push(current_push_idx)) &&
                !push_idx.compare_exchange_strong(current_push_idx, (current_push_idx + 1) % QueueCapacity));
        if (!flag) return;
        data[current_push_idx] = val;
        const int stale = current_push_idx;
        while (!shadow_push_idx.compare_exchange_strong(current_push_idx, (current_push_idx + 1) % QueueCapacity)) {
            current_push_idx = stale;
        }
    }

    std::optional<T> pop() {
        int current_pop_idx = pop_idx.load();
        bool flag;
        while ((flag = can_pop(current_pop_idx)) &&
                !pop_idx.compare_exchange_strong(current_pop_idx, (current_pop_idx + 1) % QueueCapacity));
        if (!flag) return std::nullopt;
        auto ans = data[current_pop_idx];
        const int stale = current_pop_idx;
        while (!shadow_pop_idx.compare_exchange_strong(current_pop_idx, (current_pop_idx + 1) % QueueCapacity)) {
            current_pop_idx = stale;
        }
        return ans;
    }
};

static void single_thread_test() {
    lockless_queue<int> queue;
    std::queue<int> pair_queue;
    for (int i = 0; i < 10; i ++ ) {
        queue.push(i);
        pair_queue.push(i);
    }
    for (int i = 0; i < 10; i ++ ) {
        auto ans1 = *(queue.pop());
        auto ans2 = pair_queue.front(); pair_queue.pop();
        assert(ans1 == ans2);
    }
}

static void multi_thread_test() {
    lockless_queue<int> queue;
    vector<std::thread> producer_list, consumer_list;
    constexpr static int thread_nr = 10;
    for (int i = 0; i < thread_nr; i ++ ) {
        producer_list.emplace_back (
            std::thread ([&queue]()
            {
                const int val = 42;
                for (;;) {
                    queue.push(val);
                }
            })
        );
    }
    for (int i = 0; i < thread_nr; i ++ ) {
        consumer_list.emplace_back (
            std::thread ([&queue]()
            {
                for (;;) {
                    const auto val = queue.pop();
                    if (val) {
                        std::cout << *val << std::endl;
                    } else {
                        std::cout << "empty" << std::endl;
                    }
                }
            })
        );
    }
    producer_list[0].join();
}

void lockless_queue_routine() {
    // single_thread_test();

    multi_thread_test();
}
