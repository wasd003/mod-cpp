#include <modcpp/base.h>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <modcpp/base.h>
#include <ostream>
#include <set>
#include <map>
#include <type_traits>
#include <unordered_set>
#include <cassert>
#include <sstream>
#include <deque>

using namespace std;

struct tss {};

void func(tss&& val) {
    cout << "rvalue ref\n";
}

void func(const tss& val) {
    cout << "const lvalue ref\n";
}

template<typename... Ts>
void func(Ts&&... args) {
    cout << "universal ref\n";
}


void test_routine() {
    tss ts;
    func(std::move(ts));
}
