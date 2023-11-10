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

        iterator operator--(int) {
            auto old = *this;
            idx -- ;
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

        iterator operator-(int delta) {
            auto res = *this;
            res.idx -= delta;
            return res;
        }

        iterator operator+(int delta) {
            auto res = *this;
            res.idx += delta;
            return res;
        }

        bool operator<(const iterator& rhs) const {
            return idx < rhs.idx;
        }

        bool operator<=(const iterator& rhs) const {
            return idx < rhs.idx;
        }

        bool operator>=(const iterator& rhs) const {
            return idx >= rhs.idx;
        }
    };

    iterator begin() {
        return iterator {0, this};
    }

    iterator end() {
        return iterator {N, this};
    }
};

template<typename Iterator>
void ministl_sort(Iterator&& begin, Iterator&& end) {
    if (begin >= end - 1) return;
    auto left = begin - 1, right = end;
    const auto pivot = *begin;

    while (left < right) {
        do left ++ ; while (*left < pivot);
        do right -- ; while (*right > pivot);
        if (left < right) std::swap(*left, *right);
    }

    right ++ ;
    ministl_sort(begin, right);
    ministl_sort(right, end);
}

void iterator_rountine() {
    Array<int, 10> arr;
    int i = 10;
    for (auto it = arr.begin(); it != arr.end(); it ++ ) {
        *it = i;
        i -- ;
    }

    for (auto it = arr.begin(); it != arr.end(); it ++ ) {
        print(*it);
    }
    print();
    ministl_sort(arr.begin(), arr.end());
    for (auto it = arr.begin(); it != arr.end(); it ++ ) {
        print(*it);
    }
}
