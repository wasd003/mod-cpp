#include <modcpp/base.h>

void if_switch() {
    std::vector<int> vec = {1, 2, 3, 4};

    if (auto itr = std::find(vec.begin(), vec.end(), 3); itr != vec.end()) {
        *itr = 4;
    }

    for (auto itr = vec.begin(); itr != vec.end(); itr ++ )
        std::cout << *itr << std::endl;
}

/**
 * 'if constexpr' indicates the condition check can be done during compilation period
 */
template<class T>
void _if_constexpr(T v) {
    if constexpr (std::same_as<decltype(v), int>) {
        std::cout << v + 1 << std::endl;
    } else {
        std::cout << v + 0.1 << std::endl;
    }
}

void if_constexper() {
    _if_constexpr(1);
    _if_constexpr(LLONG_MAX - 1);
}


void initializer_list_example() {
    class Foo {
    private:
        std::vector<int> vec;
    public:
        /**
         * Use initializer_list in constructor
         */
        Foo(std::initializer_list<int> list) {
            for (auto it : list) {
                vec.push_back(it);
            }
        }

        /**
         * Use initializer_list in normal function
         */
        void print(std::initializer_list<int> list) {
            for (auto it : list) {
                std::cout << it << " ";
            }
        }

    };

    /* First way to use initializer_list in constructor */
    Foo foo = {1, 2, 3, 4, 5};
    foo.print({2, 3, 4});

    /* Second way to use initializer_list in constructor */
    Foo foo_2 {3, 4};
}

std::tuple<int, double, std::string> tuple_func() {
    return std::make_tuple(1, 0.5, "Hello");
}

void structure_binding() {
    auto [x, y, z] = tuple_func();
    std::cout << x << " " << y << " " << z << std::endl;
}

int foo(int a, int b, int c) {
    return a + b + c;
}

void bind_placeholder() {
    auto foo_bind = std::bind(foo, 10, std::placeholders::_1, 30);
    std::cout << foo_bind(10) << std::endl;
}

void raw_string() {
    std::string str = R"(C:\Path\To\File)";
    std::cout << str << std::endl;
}

int likely_example(int a, int b)
{
    if (a < b) [[unlikely]] {
        return a;
    }
    return b;
}

template<class T>
void print_seq(std::span<T> s) {
    for (auto &i : s) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void span_example() {
    int a[] = {1, 2, 3};
    std::array<int, 3> c = {4, 5, 6};
    std::vector<int> b = {7, 8, 9};

    print_seq<int>(a);
    print_seq<int>(b);
    print_seq<int>(c);
}


void misc_routine() {
    if_switch();

    if_constexper();

    initializer_list_example();
    
    structure_binding();

    bind_placeholder();

    raw_string();

    likely_example(3, 5);

    span_example();
}
