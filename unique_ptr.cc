#include <modcpp/base.h>

template<typename... Ts>
void print(const Ts&... args) {
    ((std::cout << args), ...);
    std::cout << std::endl;
}

template<typename T>
class unique_pointer {
private:
    T *data;

public:
    unique_pointer(T* _data) : data(_data) {}

    unique_pointer(const unique_pointer<T>& rhs) = delete;

    unique_pointer& operator=(const unique_pointer<T>& rhs) = delete;

    unique_pointer(unique_pointer<T>&& rhs) noexcept : data(rhs.data) {
        rhs.data = nullptr;
    }

    unique_pointer& operator=(unique_pointer<T>&& rhs) noexcept {
        if (this == &rhs) return *this;
        data = rhs.data;
        rhs.data = nullptr;
        return *this;
    }

    T* operator->() {
        return data;
    }

    T& operator*() {
        return *data;
    }
};

struct test_struct {
    int field_a, field_b;
    test_struct() : field_a(24), field_b(42) {}
};

void pass_up_by_ref(unique_pointer<test_struct>& up) {
    print(up->field_a, " ", up->field_b);
}

void pass_up_by_value(unique_pointer<test_struct>&& up) {
    print(up->field_a, " ", up->field_b);
}

void unique_ptr_routine() {
    auto ptr = new test_struct();
    auto up = unique_pointer<test_struct>(ptr);
    auto up2 = std::move(up);
    pass_up_by_ref(up2);
    pass_up_by_value(std::move(up2));
}
