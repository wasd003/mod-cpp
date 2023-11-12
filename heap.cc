#include <modcpp/base.h>
#include <stdexcept>
#include <algorithm>

template<typename T, size_t MaxHeapSize = 1000>
class MinHeap {
private:
    T data[MaxHeapSize];
    int last;

    void down(int root) {
        int min_val = data[root];
        int min_val_pos = root;
        if (root * 2 <= last && data[root * 2] < min_val) {
            min_val = data[root * 2];
            min_val_pos = root * 2;
        }
        if (root * 2 + 1 <= last && data[root * 2 + 1] < min_val) {
            min_val = data[root * 2 + 1];
            min_val_pos = root * 2 + 1;
        }
        if (min_val_pos != root) {
            std::swap(data[root], data[min_val_pos]);
            down(min_val_pos);
        }
    }

    void up(int root) {
        if (root == 1) return;
        int father = root / 2;
        if (data[father] > data[root]) {
            std::swap(data[father], data[root]);
            up(father);
        }
    }

    void build_heap() {
        for (int i = last / 2; i >= 1; i -- ) down(i);
    }

    bool full() {
        return last >= MaxHeapSize - 1;
    }

    bool empty() {
        return last == 0;
    }

public:
    MinHeap() : last(0) {}

    MinHeap(const std::vector<T>& nums) {
        int n = nums.size();
        if (n > MaxHeapSize) throw std::runtime_error("nums capacity overflow");
        for (int i = 1; i <= n; i ++ ) data[i] = nums[i];
        last = n;
        build_heap();
    }

    void push(const T& val) {
        if (full()) throw std::runtime_error("heap is full");
        data[ ++ last ] = val;
        up(last);
    }

    T top() {
        if (empty()) throw std::runtime_error("heap is empty");
        return data[1];
    }

    void pop() {
        if (empty()) throw std::runtime_error("heap is empty");
        std::swap(data[1], data[last]);
        last -- ;
        down(1);
    }

    size_t size() {
        return last;
    }

};

void heap_routine() {
    auto nums = std::vector<int> {2, 3,4, 2, 8,5, 3, 5, 6};
    MinHeap<int> heap(nums);
    heap.push(10);
    heap.push(-2);
    while (heap.size()) {
        std::cout << heap.top() << " "; 
        heap.pop();
    }
}
