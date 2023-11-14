#include <modcpp/base.h>
#include <setjmp.h>
#include <cassert>
#include <source_location>

template<typename... Ts>
void print(const Ts&... args) {
    ((std::cout << args), ...);
}

jmp_buf main_ctx;
class Coroutine;
void __entry(Coroutine *coro);
void __exit(Coroutine *coro);

void switch_to(jmp_buf& from, jmp_buf& to) {
    int ret = setjmp(from);
    if (!ret) {
        longjmp(to, 1);
    }
}

class Coroutine {
private:
    using Handle = void (*)(Coroutine *);
    constexpr static int StackSize = 4096;

public:
    Handle handler;
    jmp_buf ctx;
    alignas(16) uint8_t stk[StackSize];

    struct CoroutineArgs {
        int cid;
    } args;

    Coroutine(Handle handler, int cid) : handler(handler) {
        uint64_t stk_base = reinterpret_cast<uint64_t>(stk);
        assert((stk_base & 15) == 0);
        uint64_t stk_top = stk_base + StackSize;
        args.cid = cid;
        int ret = setjmp(main_ctx);
        if (ret == 1) return;
        asm volatile ("movq %0, %%rsp\n\t"
                      "movq %1, %%rdi\n\t"
                      "pushq %2\n\t"
                      "jmp *%3\n\t"
                      :
                      : "b"(stk_top), "d"(this), "a"(__exit), "c"(__entry)
                      : "memory");
    }
};

void __entry(Coroutine *coro) {
    printf("coro-%d __entry...\n", coro->args.cid);
    switch_to(coro->ctx, main_ctx);

    printf("coro-%d working...\n", coro->args.cid);
    // pass coro parameter to __exit
    asm volatile ("movq %0, %%rdi\n\t" ::"a"(coro):"memory");
}

void __exit(Coroutine *coro) {
    printf("exit\n");
    switch_to(coro->ctx, main_ctx);
}

void coroutine_test() {
    Coroutine co0(__entry, 0);
    switch_to(main_ctx, co0.ctx);
    printf("back to main\n");
}
