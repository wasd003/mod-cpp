#include <functional>
#include <modcpp/base.h>
#include <new>
#include <string>

void do_func(const std::function<int(const std::string&)>& func) {
    func("abc");
}


void pass_lambda_to_function() {
    auto func = [](const std::string& str) {
        auto len = static_cast<int>(str.size());
        std::cout << "len:" << len << std::endl;
        return 0;
    };
    do_func(func);
}

void generalized_capture() {
    class A {
        int a, b, c;
        std::vector<int> vec;
        void func() {
            auto lambda = [a = a, vec = std::move(vec)]() mutable {
                a ++ ;
                vec.push_back(1);
            };
            lambda();
        };
    };
}

void target_value(const std::string& str) {
    std::cout << "lvalue" << std::endl;
}

void target_value(std::string&& str) {
    std::cout << "rvalue" << std::endl;
}

template<typename T>
void func(T&& args) {
    auto lambda = [&args]() {
        target_value(std::forward<T>(args));
    };
    lambda();
}

void capture_universal_reference() {
    std::string str = "wdf";
    func(str);
    func("abc");
}

void universal_parameters() {
    auto lambda = [](auto&& args) {
        target_value(std::forward<decltype(args)>(args));
    };
    std::string str = "abc";
    lambda(str);
    lambda("abc");
}

void lambda_routine() {
    universal_parameters();
    
    /* capture_universal_reference(); */

    /* pass_lambda_to_function(); */

    /* generalized_capture(); */
}
