#include <modcpp/base.h>
#include <cassert>
#include <stack>
#include <memory>
#include <thread>
#include <atomic>

using namespace std;

template<typename T>
class lockless_stack {
private:
    struct node {
        T data;
        node* prev;
        node() : prev(nullptr) {}
        node(const T& data) : data(data), prev(nullptr) {}
    };

    std::atomic<node*> head;

public:
    lockless_stack() {
        head.store(new node());
    }

    void push(const T& data) {
        node *new_node = new node(data);
        node *cur_head = head.load();
        new_node->prev = cur_head;

        while (!head.compare_exchange_strong(cur_head, new_node)) {
            new_node->prev = cur_head;
        }
    }

    T* pop() {
        node *cur_head = head.load();
        while (cur_head->prev && !head.compare_exchange_strong(cur_head, cur_head->prev));
        if (!cur_head->prev) return nullptr;
        auto ans = new T(cur_head->data);
        delete cur_head;
        return ans;
    }
};

void single_thread_test() {
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

void multi_thread_test() {
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
    single_thread_test();

    multi_thread_test();
}
