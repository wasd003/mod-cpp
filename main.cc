#include <modcpp/base.h>

void misc_routine();
void auto_routine();
void template_routine();
void oo_routine();
void rvalue_routine();
void lockless_stack_routine();
void future_routine();
void operator_rountine();
void smart_pointer_routine();
void lambda_routine();
void test_rng();
void coro_main();
void keyword_routine();
void exception_routine();
void design_pattern_routine();
void linked_list_routine();
void vec_routine();

int main() {

    lockless_stack_routine();

#if 0
    future_routine();

    vec_routine();

    linked_list_routine();

    design_pattern_routine();

    exception_routine();

    lambda_routine();

    smart_pointer_routine();

    template_routine();

    oo_routine();

    misc_routine();

    auto_routine();

    rvalue_routine();
    

    operator_rountine();

    coro_main();

    keyword_routine();
#endif
}
