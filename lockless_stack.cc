#include <modcpp/base.h>
#include <memory>
#include <thread>

using namespace std;

template<typename T>
class lockless_stack {
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::shared_ptr<node> next;
        node(T const& data_):
            data(std::make_shared<T>(data_))
        {}
    };
    std::shared_ptr<node> head;
public:
    void push(T const& data)
    {
        auto new_node = make_shared<node>(data);
        new_node->next = std::atomic_load(&head);
        while (!std::atomic_compare_exchange_weak(&head, &new_node->next, new_node));
    }
    std::shared_ptr<T> pop()
    {
        auto old_node = std::atomic_load(&head);
        while (old_node && !std::atomic_compare_exchange_weak(&head, &old_node,
                    std::atomic_load(&old_node->next)));
        if (old_node) {
            // TODO: is it necessary?
            std::atomic_store(&old_head->next,std::shared_ptr<node>());
            return old_node->data;
        } else {
            return std::make_shared<T>();
        }
    }
    ~lockless_stack(){
        while (pop());
    }
};

void lockless_stack_routine() {
    lockless_stack<int> stk;
    vector<std::thread> producer_list, consumer_list;
    for (int i = 0; i < 2; i ++ ) {
        producer_list.emplace_back (
            std::thread ([&stk]()
            {
                int i = 0;
                for (;;) {
                    stk.push(i);
                    i ++ ;
                }
            })
        );
    }
    for (int i = 0; i < 2; i ++ ) {
        consumer_list.emplace_back (
            std::thread ([&stk]()
            {
                for (;;) {
                    auto val = stk.pop();
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
