#include <modcpp/base.h>
#include <cassert>
#include <stack>
#include <memory>
#include <thread>
#include <atomic>
#include <optional>

using namespace std;

template<typename T>
class lockless_stack {
private:
    struct node {
        node() : prev(nullptr) {}
        node(const T& data) : prev(nullptr), data(data) {}
        node *prev;
        T data;
    };
    std::atomic<node*> head;
public:
    lockless_stack() : head(nullptr) {}

    void push(const T& data ) {
        auto new_node = new node(data);
        auto cur_head = head.load();
        new_node->prev = cur_head;
        while (!head.compare_exchange_weak(cur_head, new_node)) {
            new_node->prev = cur_head;
        }
    }

    std::optional<T> pop() {
        auto cur_head = head.load();
        while (cur_head && !head.compare_exchange_weak(cur_head, cur_head->prev));
        if (!cur_head) return std::nullopt;
        auto ans = cur_head->data;
        delete cur_head;
        return ans;
    }
};

static void single_thread_test() {
    lockless_stack<int> stk;
    std::stack<int> pair_stk;
    for (int i = 0; i < 10; i ++ ) {
        stk.push(i);
        pair_stk.push(i);
    }

    for (int i = 0; i < 10; i ++ ) {
        auto ans1 = *(stk.pop());
        auto ans2 = pair_stk.top(); pair_stk.pop();
        assert(ans1 == ans2);
    }
}

static void multi_thread_test() {
    lockless_stack<int> stk;
    vector<std::thread> producer_list, consumer_list;
    constexpr static int thread_nr = 10;
    for (int i = 0; i < thread_nr; i ++ ) {
        producer_list.emplace_back (
            std::thread ([&stk]()
            {
                const int val = 42;
                for (;;) {
                    stk.push(val);
                }
            })
        );
    }
    for (int i = 0; i < thread_nr; i ++ ) {
        consumer_list.emplace_back (
            std::thread ([&stk]()
            {
                for (;;) {
                    const auto val = stk.pop();
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

void lockless_stack_routine() {
    // single_thread_test();

    multi_thread_test();
}
