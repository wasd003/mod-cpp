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

void pass_unique_ptr_by_ref(std::unique_ptr<std::vector<int>>& vecp) {
    vecp->push_back(1);
}

void pass_unique_ptr_by_value(std::unique_ptr<std::vector<int>> vecp) {
    vecp->push_back(2);
    auto& vec = *vecp;
    for (auto &i : vec) {
        std::cout << i << " " << std::endl;
    }
}

void pass_unique_ptr_example() {
    auto vecp = std::make_unique<std::vector<int>>();
    vecp->push_back(10);
    pass_unique_ptr_by_ref(vecp);
    pass_unique_ptr_by_value(std::move(vecp));
    assert(vecp.get() == nullptr);
}

void smart_pointer_routine() {
    pass_unique_ptr_example();
    /* weak_pointer(); */
}
