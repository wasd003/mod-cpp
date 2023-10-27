#include <source_location>
#include <modcpp/base.h>

template<class T>
using TDVec = std::vector<std::vector<T>>;
void template_alias() {
    TDVec<int> v;
    v.push_back({1, 2, 3});
    v.push_back({4, 5, 6});
    for (auto i : v)
        for (auto j : i)
            std::cout << j << " " << std::endl;
}

template<typename T, typename... Ts>
void custom_printf(T t0, Ts... ts) {
    // | T     | t0       | sizeof    |
    // | Ts... | ts.../ts | sizeof... |
    std::cout << t0 << std::endl;
    if constexpr (sizeof...(ts) > 0) {
        custom_printf(ts...);
    }
}

void variadic_template() {
    custom_printf(1, "Hello", 0.5);
}

/* template<int param> is also ok */
template<auto param>
void speak() {
    std::cout << param << std::endl;
}

/*
 * use template as a param instead of type
 */
void non_type_template() {
    speak<100>();
}

template<typename... Ts>
void fold_print(const Ts&... args) {
    ((std::cout << args << " "), ...);
}

template<typename Ta, typename... Tb>
decltype(auto) fold_sum(const Ta& first, const Tb&... args) {
    return (first + ... + args);
}

void template_fold() {
    auto ans = fold_sum(1, 2, 3.5);
    std::cout << ans << std::endl;
    fold_print("chs", 2, 3);
}

template<typename T, typename U>
class A {
public:
    A() {
        std::cout << "no specialization" << std::endl;
    }
};

template<typename U>
class A<int, U> {
public:
    A() {
        std::cout << "partial specialization" << std::endl;
    }

};

template<>
class A<int, char> {
public:
    A() {
        std::cout << "full specialization" << std::endl;
    }
};

void class_template_specialization() {
    A<char, char> a1;
    A<int, double> a2;
    A<int, char> a3;
}

template<typename T>
void tmpl_func() {
    std::cout << "no specialization" << std::endl;
}

template<>
void tmpl_func<char>() {
    std::cout << "full specialization" << std::endl;
}

void func_template_specialization() {
    tmpl_func<int>();
    tmpl_func<char>();
}

template <typename T>
class Base {
public:
    struct Nested {
        int val;
        Nested& operator+(const Nested& rhs) {
            val += rhs.val;
            return *this;
        }
        friend std::ostream& operator<<(std::ostream& os, const Nested& rhs) {
            os << rhs.val;
            return os;
        }
    };
    void base_func() {
        const auto &loc = std::source_location::current();
        std::cout << loc.function_name() << ":" << loc.line() << std::endl;
    }
};

template <typename T>
class Derived : public Base<T> {
public:
    void derived_func() {
        // 1. use function in template base class
        this->base_func();

        // 2. use nested type defined in template base class
        using nested = typename Base<T>::Nested;
        nested x = {1};
        nested y = {3};
        auto z = x + y;
        std::cout << z << std::endl;
    }
};

void derived_template_example() {
    Derived<int> d;
    d.derived_func();
}

void template_routine() {
#if 0
    class_template_specialization();

    func_template_specialization();

    template_alias();

    variadic_template();

    non_type_template();

    template_fold();
#endif
    derived_template_example();
}
