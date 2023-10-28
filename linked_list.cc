#include <cassert>
#include <memory>
#include <modcpp/base.h>

using namespace std;

template<typename T>
class linked_list {
private:
    struct node {
        node() = default;
        node(const T& data) : data(data) {}

        T data;
        shared_ptr<node> next;
        weak_ptr<node> prev;
    };

    shared_ptr<node> head;

public:
    linked_list() : head(make_shared<node>()) {
        head->next = head;
        head->prev = head;
    }

    void push_back(const T& data) {
        shared_ptr<node> new_node = make_shared<node>(data);
        shared_ptr<node> tail = head->prev.lock();
        new_node->prev = tail;
        tail->next = new_node;
        new_node->next = head;
        head->prev = new_node;
    }

    T pop_back() {
        if (empty()) throw runtime_error("list is empty");
        auto tail = head->prev.lock();
        auto second_tail = tail->prev.lock();
        head->prev = second_tail;
        second_tail->next = head;
        return tail->data;
    }

    void push_front(const T& data) {
        auto new_node = make_shared<node>(data);
        auto second_head = head->next;
        new_node->prev = head;
        head->next = new_node;
        new_node->next = second_head;
        second_head->prev = new_node;
    }

    T pop_front() {
        if (empty()) throw runtime_error("list is empty");
        auto second_head = head->next;
        auto third_head = second_head->next;
        head->next = third_head;
        third_head->prev = head;
        return second_head->data;
    }

    bool empty() {
        return head->prev.lock() == head;
    }
};

void linked_list_routine() {
    linked_list<int> list;
    for (int i = 0; i < 10; i ++ ) {
        list.push_back(i);
    }
    for (int i = 0; i < 10; i ++ ) {
        auto ans = list.pop_front();
        assert(ans == i);
    }
    assert(list.empty());
    std::cout << "All Tests Passed :)" << std::endl;
}
