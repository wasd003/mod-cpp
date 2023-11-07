#include <modcpp/base.h>
#include <cassert>
#include <source_location>

template<typename... T>
void print(const T&... args) {
    ((std::cout << args), ...);
    std::cout << std::endl;
}

void do_trace(const std::source_location& loc = std::source_location::current()) {
    print(loc.function_name(), ":", loc.line());
}

class COW {
private:
    struct Data {
        constexpr static int N = 100;
        std::vector<int> vec;
        int refcnt;

        Data() : refcnt(1) {}
        Data(std::vector<int>&& vec) : refcnt(1), vec(std::move(vec)) {
            do_trace();
        }
        Data(const std::vector<int>& vec) : refcnt(1), vec(vec) {
            do_trace();
        }
        Data(const Data& rhs) : refcnt(1), vec(rhs.vec) {}
        Data& operator=(const Data& rhs) = delete;
        ~Data() = default;
    };

    Data *data;

    void decrese_refcnt(Data *data) {
        if ( -- data->refcnt == 0) {
            delete data;
            data = nullptr;
        }
    }
public:
    COW() : data(nullptr) {}

    template<typename... Ts>
    COW(Ts&&... args) {
        data = new Data(std::forward<Ts>(args)...);
    }

    COW(const COW& rhs) : data(rhs.data) {
        do_trace();
        data->refcnt ++ ;
    }

    COW& operator=(const COW& rhs) {
        do_trace();
        if (this == &rhs) [[unlikely]] return *this;
        decrese_refcnt(data);
        data = rhs.data;
        data->refcnt ++ ;
        return *this;
    }

    COW(COW&& rhs) : data(rhs.data) {
        do_trace();
    }

    COW& operator=(COW&& rhs) {
        do_trace();
        assert(this != &rhs);
        decrese_refcnt(data);
        data = rhs.data;
        return *this;
    }

    ~COW() {
        decrese_refcnt(data);
    }

    void mutable_operation() {
        if (data->refcnt != 1) {
            auto new_data = new Data(*data);
            decrese_refcnt(data);
            data = new_data;
        }
        assert(data->refcnt == 1);
    }

    void readonly_operation() const {
        // ...
    }
};

void cow_routine() {
    COW origin_obj (std::vector<int> {1, 2, 3});
    COW new_obj = std::move(origin_obj);
}
