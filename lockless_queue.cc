#include <modcpp/base.h>
#include <cassert>
#include <queue>
#include <memory>
#include <thread>
#include <atomic>

using namespace std;

template<typename T>
class lockless_queue {
private:
    constexpr static int capacity = 4096;

    std::atomic<int> push_idx, can_consume_idx;
    std::atomic<int> pop_idx,  can_push_idx;

    struct node {
        T data;
        node() = default;
        node(const T& data) : data(data)  {}
    } queue[capacity];

    bool is_full(int cur_push_idx) {
        return (cur_push_idx + 1) % capacity == can_push_idx;
    }

    bool is_empty(int cur_pop_idx) {
        return cur_pop_idx == can_consume_idx;
    }

public:
    lockless_queue() : push_idx(0), can_consume_idx(0), pop_idx(0), can_push_idx(0) {}

    void push(const T& data) {
        int cur_push_idx = push_idx.load();
        bool is_full_ret;
        while (!(is_full_ret = is_full(cur_push_idx)) &&
                !push_idx.compare_exchange_strong(cur_push_idx, (cur_push_idx + 1) % capacity));
        if (is_full_ret) return;
        queue[cur_push_idx] = data;
        while (can_consume_idx != cur_push_idx);
        can_consume_idx = (can_consume_idx + 1) % capacity;
    }

    T* pop() {
        int cur_pop_idx = pop_idx.load();
        bool is_empty_ret;
        while (!(is_empty_ret = is_empty(cur_pop_idx)) &&
                !pop_idx.compare_exchange_strong(cur_pop_idx, (cur_pop_idx + 1) % capacity));
        if (is_empty_ret) return nullptr;
        auto ans = new T(queue[cur_pop_idx].data);
        while (can_push_idx != cur_pop_idx);
        can_push_idx = (can_push_idx + 1) % capacity;
        return ans;
    }
};

static void single_thread_test() {
    lockless_queue<int> queue;
    std::queue<int> pair_queue;
    for (int i = 0; i < 10; i ++ ) {
        auto ans1 = *(queue.pop());
        auto ans2 = pair_queue.front(); pair_queue.pop();
        assert(ans1 == ans2);
    }

    for (int i = 0; i < 10; i ++ ) {
        queue.push(i);
        pair_queue.push(i);
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
    for (int i = 0; i < producer_list.size(); i ++ )
        producer_list[i].join();
    for (int i = 0; i < consumer_list.size(); i ++ )
        consumer_list[i].join();
}

void lockless_queue_routine() {
    single_thread_test();

    multi_thread_test();
}
