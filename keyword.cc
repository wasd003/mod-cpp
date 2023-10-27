#include <array>
#include <modcpp/base.h>

void mutable_example() {
    struct A {
        mutable int x = 0;
        void foo() const {
            x ++ ;
        }
    };
    A a;
    a.foo();
    std::cout << a.x << std::endl;
}

void diff_const_vs_constexpr_example() {
    int x;
    std::cin >> x;
    const int a = x; // OK
#if 0
    constexpr int b = x; // Wrong, not known during compilation time
#endif
}

template<auto N>
void non_typename_template() {
    std::cout << N << std::endl;
}

template<auto N>
consteval auto filter_primes() {
    std::array<int, N + 1> is_primes;
    is_primes.fill(true);
    for (int i = 2; i <= N; i ++ ) {
        if (!is_primes[i]) continue;
        for (int j = i + i; j <= N; j += i)
            is_primes[j] = false;
    }
    is_primes[1] = false;
    return is_primes;
}

auto compute_at_compilation() {
    constexpr auto is_primes = filter_primes<100>();
    return is_primes;
}

void type_traits_example() {
    const int x = 10;

    static_assert(std::is_integral_v<decltype(x)>);

    static_assert(std::is_same_v<int, std::remove_const_t<decltype(x)>>);
}

void keyword_routine() {
    /* mutable_example(); */

    /* diff_const_vs_constexpr_example(); */

    /* non_typename_template<12>(); */

    /* compute_at_compilation(); */

    type_traits_example();
}
