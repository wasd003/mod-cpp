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

int main() {

#if 0
    smart_pointer_routine();

    template_routine();

    misc_routine();

    auto_routine();

    oo_routine();

    rvalue_routine();
    
    lockless_stack_routine();

    future_routine();

    lambda_routine();
#endif
    operator_rountine();
}
