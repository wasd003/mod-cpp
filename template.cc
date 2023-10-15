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

void template_routine() {
    template_alias();

    variadic_template();

    non_type_template();

    template_fold();
}
