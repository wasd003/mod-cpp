#include <source_location>
#include <cassert>
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

class A {

private:
    constexpr static int N = 100100;
    std::array<int, N> arr;

public:
    A() {
        std::cout << "default constructor" << std::endl;
    }

    A(const A& rhs) {
        std::cout << "copy constructor" << std::endl;
        arr = rhs.arr;
    }

    int& operator[](int idx) {
        assert(idx >= 0 && idx < arr.size());
        return arr[idx];
    }

    A& operator=(const A& rhs) {
        std::cout << "assignment operator" << std::endl;
        arr = rhs.arr;
        return *this;
    }

    A(A&& rhs) {
        std::cout << "move constructor" << std::endl;
        arr = std::move(rhs.arr);
    }

    A& operator=(A&& rhs) {
        std::cout << "move operator" << std::endl;
        arr = std::move(rhs.arr);
        return *this;
    }
};


auto func_with_A = [](const A& args) {
    const auto& loc = std::source_location::current();
    std::cout << loc.function_name() << ":" << loc.line() << std::endl;
};

void copy_elission() {
    A a = A();
    func_with_A(A());

    auto V = std::vector<int>();
}

struct example_for_static_member {
    constexpr static int idx = 10; // OK, const static member
    static std::vector<int> v; // need to init outside class
    static struct static_constructor {
        static_constructor() {
            for (int i = 0; i < 10; i ++ )
                v.push_back(i);
        }
    } _static_constructor;
};

std::vector<int> example_for_static_member::v;

example_for_static_member::static_constructor
    example_for_static_member::_static_constructor;

void init_static_member() {
    auto& vec = example_for_static_member::v;
    while (vec.size()) {
        auto x = vec.back();
        vec.pop_back();
        std::cout << x << std::endl;
    }
}

class widget {
private:
    A data;
public:
    A get_data() & {
        return data;
    }
    A get_data() && {
        return std::move(data);
    }
};

void reference_qualifier() {
    A data = widget().get_data();
}

class B : public A {
private:
    int a, b, c;

public:
    B(int a, int b, int c) : A(), a(a), b(b), c(c) {}

    B& operator=(const B& rhs) {
        if (this == &rhs) [[unlikely]] {
            return *this;
        }
        A::operator=(rhs);
        a = rhs.a;
        b = rhs.b;
        c = rhs.c;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const B& rhs) {
        os << rhs.a << " " << rhs.b << " " << rhs.c;
        return os;
    }
};

void derived_assignment_operator() {
    B b1(1, 2, 3);
    B b2(2, 3, 4);
    std::cout << b1 << std::endl;
    b1 = b2;
    std::cout << b1 << std::endl;
}

void virtual_inheritence() {
    class common_base {

    };

    class base_1 : virtual public common_base {

    };

    class base_2 : virtual public common_base {

    };

    class derived : public base_1, public base_2 {

    };
}

void oo_routine() {
#if 0
    delegate_and_inheritance_constructor();

    explicit_override_virt_func();

    copy_elission();

    init_static_member();

    reference_qualifier();
#endif
    derived_assignment_operator();
}
