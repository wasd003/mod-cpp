#include <modcpp/base.h>
#include <source_location>

template<typename... Ts>
void print(const Ts&... args) {
    ((std::cout << args), ...);
    std::cout << std::endl;
}

static void do_trace(const std::source_location& loc = std::source_location::current()) {
    print(loc.function_name(), ":", loc.line());
}

template<typename T, auto N>
class Array {
private:
    T data[N];

public:
    class iterator {
    private:
        int idx;
        Array<T, N> *self;
    public:
        iterator(int idx, Array<T, N> *arr) : idx(idx), self(arr) {}

        iterator& operator++() {
            idx ++ ;
            return *this;
        }

        iterator operator++(int) {
            auto old = *this;
            idx ++ ;
            return old;
        }

        T* operator->() {
            return &self->data[idx];
        }

        T& operator*() {
            return self->data[idx];
        }

        bool operator==(const iterator& rhs) const {
            return idx == rhs.idx && self == rhs.self;
        }

        bool operator!=(const iterator& rhs) const {
            return idx != rhs.idx || self != rhs.self;
        }
    };

    iterator begin() {
        return iterator {0, this};
    }

    iterator end() {
        return iterator {N, this};
    }
};

void iterator_rountine() {
    Array<int, 10> arr;
    auto iter = arr.begin();
    *iter = 10;
    int i = 0;
    for (auto it = arr.begin(); it != arr.end(); it ++ ) {
        print("i:", i);
        *iter = i;
        i ++ ;
    }
}
