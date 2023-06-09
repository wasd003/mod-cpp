#include <modcpp/base.h>

void delegate_and_inheritance_constructor() {
    class Base {
    public:
        int foo;
        int bar;
        Base() : foo(1) {}
        Base(int _bar) : Base() // delegate constructor
        {
            bar = _bar;
        }
    };

    class Sub : public Base {
    public:
        using Base::Base; // inheritence constructor
    };

    Sub sub(2);
    std::cout << sub.foo << " " << sub.bar << std::endl;
}

void explicit_override_virt_func() {
    class Base {
    public:
        virtual void foo() {
            std::cout << "foo in Base" << std::endl;
        }
    };

    class Sub : public Base {
    public:
        void foo() override // use 'override' to explict rewrite virt func in Base Class
        {
            std::cout << "foo in Sub" << std::endl;
        }
    };

    Sub sub;
    sub.foo();

}

void oo_routine() {
    delegate_and_inheritance_constructor();

    explicit_override_virt_func();
}
