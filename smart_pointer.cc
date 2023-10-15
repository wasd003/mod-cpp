#include <cassert>
#include <memory>
#include <modcpp/base.h>

class A {
public:
    int a, b, c;
};

void weak_pointer() {
    auto sp = std::make_shared<A>(1, 2, 3);
    std::weak_ptr<A> wp = sp;
    sp.reset();
    if (wp.expired()) {
        std::cout << "Expired!" << std::endl;
    } else {
        auto ssp = wp.lock();
        assert(ssp != nullptr);
        std::cout << ssp->a << std::endl;
    }
}

void smart_pointer_routine() {
    weak_pointer();
}
