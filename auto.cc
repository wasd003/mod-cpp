#include <modcpp/base.h>

void auto_as_param() {
    auto auto_add = [](auto a, auto b) {
        return a + b;
    };
    std::cout << auto_add(1, 1) << std::endl;
}

/* decltype is used to extract type info from varible */
void decltype_example() {
    int a = 1;
    decltype(a) b;
    
    static_assert(std::same_as<decltype(b), int>, "ERR");
}

/* return type inference in c++11 */
template<class T, class U>
auto add_infer_cpp11(T a, U b) -> decltype(a+b) {
    return a + b;
}

/* return type inference in c++14 */
/* return type is inferenced via auto's rule */
template<class T, class U>
auto add_infer_cpp14(T a, U b) {
    return a + b;
}

/* return type is identical to typeof(a+b) */
/* no type inference is executed */
template<class T, class U>
decltype(auto) add_exact(T a, U b) {
    return a + b;
}

void auto_as_return_type() {
    std::cout << add_infer_cpp11<int, double>(1, 1.5) << std::endl;

    std::cout << add_infer_cpp14<int, double>(2, 1.5) << std::endl;

    std::cout << add_exact<int, double>(2, 1.5) << std::endl;
}

void auto_routine() {
    auto_as_param();

    decltype_example();

    auto_as_return_type();
}
