#include <functional>
#include <modcpp/base.h>

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

void lambda_routine() {
    pass_lambda_to_function();
}
