#include <modcpp/base.h>

void misc_routine();
void auto_routine();
void template_routine();
void oo_routine();
void rvalue_routine();
void lockless_stack_routine();
void future_routine();
void operator_rountine();

int main() {
    template_routine();

#if 0
    misc_routine();

    auto_routine();


    oo_routine();

    rvalue_routine();
    
    lockless_stack_routine();

    future_routine();

    operator_rountine();
#endif
}
