#include <modcpp/base.h>
#include <new>

void my_new_handler() {
    std::cout << "handle nothing\n";
}

// this function has no functionality
// just to intercept set_new_handler for print
std::new_handler set_new_handler_wrapper(std::new_handler new_handle) {
    std::cout << "set_new_handler:" << new_handle << std::endl;
    return std::set_new_handler(new_handle);
}

class new_handler_wrapper {
private:
    std::new_handler old_handle;
public:
    new_handler_wrapper(std::new_handler handle) :
        old_handle(set_new_handler_wrapper(handle)) {}
    new_handler_wrapper(const new_handler_wrapper&) = delete;
    new_handler_wrapper(new_handler_wrapper&&) = delete;
    new_handler_wrapper& operator=(const new_handler_wrapper&) = delete;
    new_handler_wrapper& operator=(new_handler_wrapper&&) = delete;
    ~new_handler_wrapper() {
        set_new_handler_wrapper(old_handle);
    }
};

template<typename T>
class custom_new_handler {
public:
    void* operator new(size_t n) {
        new_handler_wrapper wrapper(my_new_handler);
        return ::operator new(n);
    }
};

class A : public custom_new_handler<A> {};
class B : public custom_new_handler<B> {};

void crtp_routine() {
    A* pa = new A();
    B* pb = new B();
}
