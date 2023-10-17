#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>

struct test_struct {
    int field_a, field_b;
    test_struct(int field_a, int field_b) : field_a(field_a), field_b(field_b) {}
    test_struct() = delete;
};

template<typename T, int N>
class vec {
private:
    /* stored on stack  */
    uint8_t stack_storage[N * sizeof (T)];

    /* stored on heap */
    uint8_t* heap_storage = nullptr;

    std::size_t len = 0;

    std::size_t capacity = N;

public:
    vec() = default;

    std::size_t size() {
        return len;
    }

    void grow() {
        assert(len >= capacity);
        assert(heap_storage != nullptr);
        capacity <<= 1;
        auto old_heap_storage = heap_storage;
        heap_storage = new uint8_t[capacity * sizeof (T)];
        // copy byte by byte
        for (std::size_t i = 0; i < len * sizeof(T); i ++ ) heap_storage[i] = old_heap_storage[i];
        delete [] old_heap_storage;
    }

    void push_back(const T& args) {
        emplace_back(std::move(args));
    }

    // case #1. if current len is < N,
    //      allocated element on stack
    // case #2. if current len is >= N
    //      case 2.1: if the heap is not initialized:
    //          a) init the heap
    //          b) copy all the elements on stack to the heap
    //      case 2.2: if the current len >= capacity
    //          a) grow current capacity
    //          b) copy all the elements to the heap area
    //      allocated element on heap
    template<typename... Ts>
    void emplace_back(Ts&&... args) {
        if (len < N) {
            new (stack_storage + len * sizeof(T)) T(std::forward<Ts>(args)...);
            len ++ ;
            std::cout << "push on stack. " << "size: " << len << " capacity: " << capacity << std::endl;
        } else {
            // case 2.1: init heap, make sure heap_storage != nullptr
            if (!heap_storage) [[unlikely]] {
                capacity <<= 1;
                heap_storage = new uint8_t[capacity * sizeof (T)];
                for (std::size_t i = 0; i < len * sizeof(T); i ++ )
                    heap_storage[i] = stack_storage[i];
            }
            // case 2.2: check whether we need to grow capacity
            if (len >= capacity) grow();

            // allocated element on heap
            new (heap_storage + len * sizeof(T)) T(std::forward<Ts>(args)...);
            len ++ ;
            std::cout << "push on heap. " << "size: " << len << " capacity: " << capacity << std::endl;
        }

    }

    T& operator[](int index) {
        uint8_t* base = !heap_storage ? stack_storage : heap_storage;
        T* tptr = reinterpret_cast<T*>(base + index * sizeof (T));
        return *tptr;
    }
};

int main() {
    const int N = 8;
    vec<test_struct, N> my_vec;
    for (int i = 0; i < 3 * N; i ++ ) {
        my_vec.emplace_back(i, i);
        auto val = my_vec[i];
        assert(val.field_a == i && val.field_b == i);
    }
    for (int i = 0; i < 3 * N; i ++ ) {
        auto val = my_vec[i];
        assert(val.field_a == i && val.field_b == i);
    }
}
