#include <cctype>
#include <stack>
#include <cstddef>
#include <exception>
#include <map>
#include <list>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <concepts>
#include <iostream>
#include <thread>
#include <memory>
#include <initializer_list>
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

class A {
private:
    int a, b, c;

public:
    A() = default;
    A(int a, int b, int c) : a(a), b(b), c(c) {}

    // x == y
    bool operator== (const A& rhs) const {
        return a == rhs.a && b == rhs.b && c == rhs.c;
    }

    // ++ x
    A& operator++ () {
        a ++ ;
        return *this;
    }

    // x ++ 
    A operator++ (int) {
        A tmp = *this;
        tmp.a ++ ;
        return tmp;
    }

    // x = y
    A& operator= (const A& rhs) {
        if (*this == rhs) [[unlikely]] return *this;
        a = rhs.a;
        b = rhs.b;
        c = rhs.c;
        return *this;
    }

    // x += y
    A& operator+= (const A& rhs) {
        a += rhs.a;
        b += rhs.b;
        c += rhs.c;
        return *this;
    }

    // x + y
    A operator+ (const A& rhs) const {
        auto res = *this;
        res.a += rhs.a;
        res.b += rhs.b;
        res.c += rhs.c;
        return res;
    }

    // <<
    friend ostream& operator<<(ostream& os, const A& rhs) {
        os << rhs.a << " " << rhs.b << " " << rhs.c;
        return os;
    }
};


void operator_rountine() {
    A a {1, 2, 3}, b {4, 5, 6};
    auto c = a + b;
    cout << c << endl;
}               
