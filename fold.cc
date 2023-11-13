#include <modcpp/base.h>

template<typename First, typename... Ts>
decltype(auto) sum(const First& first, const Ts&... args) {
    return (first + ... + args);
}

template<typename Last>
decltype(auto) recursive_sum(const Last& last) {
    return last;
}

template<typename First, typename... Ts>
decltype(auto) recursive_sum(const First& first, const Ts&... args) {
    return first + recursive_sum(args...);
}

template<typename Last>
void recursive_print(const Last& last) {
    std::cout << last << std::endl;
}

template<typename First, typename... Ts>
void recursive_print(const First& first, const Ts&... args) {
    std::cout << typeid(first).name() << " ";
    recursive_print(args...);
}
void fold_routine() {
    auto ans = recursive_sum(1, 2, 3.5);
    std::cout << ans << std::endl;

    recursive_print(100, 2, 3.5);
}


