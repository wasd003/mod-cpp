#include <modcpp/base.h>

class eagar_singleton {
private:
    eagar_singleton() {}
    static eagar_singleton instance;
public:
    static eagar_singleton get_instance() {
        return instance;
    }
};

class lazy_singleton {
private:
    lazy_singleton() {}

public:
    static lazy_singleton get_instance() {
        static lazy_singleton instance;
        return instance;
    }
};

void singleton() {
    auto es = eagar_singleton::get_instance();
    auto ls = lazy_singleton::get_instance();
}

void design_pattern_routine() {
    singleton();
}
