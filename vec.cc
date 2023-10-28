#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <utility>

struct test_struct {
    int field_a, field_b;

    test_struct(const test_struct& rhs) {
        static int cnt = 0;
        cnt ++ ;
        std::cout << "copy constructor:" << cnt <<  std::endl;
        field_a = rhs.field_a;
        field_b = rhs.field_b;
    }

    test_struct& operator=(const test_struct& rhs) {
        std::cout << "assignment operator" << std::endl;
        field_a = rhs.field_a;
        field_b = rhs.field_b;
        return *this;
    }

    test_struct(test_struct&& rhs) {
        std::cout << "move constructor" << std::endl;
        field_a = rhs.field_a;
        field_b = rhs.field_b;
    }

    test_struct& operator=(test_struct&& rhs) {
        std::cout << "move assignment operator" << std::endl;
        field_a = rhs.field_a;
        field_b = rhs.field_b;
        return *this;
    }

    test_struct(int fa, int fb) : field_a(fa), field_b(fb) {
        std::cout << "normal constructor" << std::endl;
    }

    ~test_struct() = default;

    test_struct() = default;
};

template<typename T, auto N>
class vec {
private:
    T stack_vec[N];
    T *heap_vec = nullptr;
    int vec_size = 0;
    int vec_capacity = N;

    /**
     * grow @vec_capacity
     * copy from stack to heap if vec_size == N
     * otherwise, copy from heap to heap
     */
    void grow() {
        std::cout << "----- begin grow -----" << std::endl;
        vec_capacity <<= 1;
        auto old_heap_vec = heap_vec;
        heap_vec = new T[vec_capacity];
        if (vec_size == N) [[unlikely]] {
            // copy from stack to heap
            for (int i = 0; i < vec_size; i ++ )
                heap_vec[i] = stack_vec[i];
        } else {
            // copy from heap to heap
            for (int i = 0; i < vec_size; i ++ )
                heap_vec[i] = old_heap_vec[i];
        }
        delete[] old_heap_vec;
        std::cout << "----- end grow -----" << std::endl;
    }

public:
    T& operator[](int idx) {
        if (vec_size <= N) return stack_vec[idx];
        else return heap_vec[idx];
    }

    template<typename...Ts>
    void emplace_back(Ts&&... args) {
        if (vec_size < N) {
            // case #1: store on stack
            new (stack_vec + vec_size ++ ) T(std::forward<Ts>(args)...);
        } else {
            // case #2: store on heap
            if (vec_size == vec_capacity) {
                grow();
            }
            // emplace construct
            new (heap_vec + vec_size ++ ) T(std::forward<Ts>(args)...);
            // move assignment
            // heap_vec[vec_size ++ ] = T(std::forward<Ts>(args)...);
        }
    }
};

void vec_routine() {
    const int N = 8;
    vec<test_struct, N> my_vec;
    for (int i = 0; i < 3 * N; i ++ ) {
        my_vec.emplace_back(i, i);
        /* auto val = my_vec[i]; */
        /* assert(val.field_a == i && val.field_b == i); */
    }
}
