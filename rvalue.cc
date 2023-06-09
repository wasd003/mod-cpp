#include <iterator>
#include <modcpp/base.h>
#include <modcpp/log.h>
#include <ostream>
#include <utility>


/**
 * 1. const lvalue ref can refer to rvalue
 * 2. rvalue ref can't refer to lvalue
 */
void basic_rule() {
    trace();
    std::string  lv1 = "string,";       // lv1 is a lvalue
    // std::string&& r1 = lv1;          // illegal, rvalue can't ref to lvalue
    std::string&& rv1 = std::move(lv1); // legal, std::move can convert lvalue to rvalue

    std::string&& rv2 = lv1 + lv1;      // legal, lv1 and lv2 are both lvalue, but lv1 + lv2 produce rvalue
                                        
    const std::string& lv2 = lv1 + lv1; // legal, const lvalue can refer to rvalue
}

void move_constructor_example1() {
    trace();
    class A {
    public:
        int *pointer;
        int val;
        /**
         * Default Constructor
         */
        A() : pointer(new int(1)), val(1) {
            std::cout << "construct: " << pointer << std::endl;
        }

        /**
         * Copy Constructor
         */
        A(A& rhs) {
            this->pointer = new int(*rhs.pointer);
            this->val = rhs.val;
            std::cout << "copy: " << pointer << std::endl;
        }

        /**
         * Move Constructor
         */
        A(A&& rhs) : pointer(rhs.pointer) {
            this->val = rhs.val;
            this->pointer = rhs.pointer;
            rhs.pointer = nullptr;
            rhs.val = 0;
            std::cout << "move: " << this->pointer << std::endl;
        }

        ~A(){
            std::cout << "destruct: " << pointer << std::endl;
            delete pointer;
        }
    };
    
    // avoid compiler optimization
    auto return_rvalue = [](bool flag = false) -> A {
        A a, b;
        if (!flag) return a;
        else return b;
    };

    A obj = return_rvalue();
    std::cout << "obj:" << std::endl;
    std::cout << obj.pointer << std::endl;
    std::cout << *obj.pointer << std::endl;
}

void move_constructor_example2() {
    trace();
    std::string str = "Hello world.";
    std::vector<std::string> v;

    v.push_back(str);
    std::cout << "str: " << str << std::endl;

    v.push_back(std::move(str));
    std::cout << "str: " << str << std::endl;
}



template <typename T>
void reference(T& v) {
    std::cout << "lvalue" << std::endl;
}

template <typename T>
void reference(T&& v) {
    std::cout << "rvalue" << std::endl;
}

template <typename T>
void not_use_forward(T&& v) {
    reference(v);
}

template <typename T>
void use_forward(T&& v) {
    reference(std::forward<T>(v));
}

void perfect_forwarding() {
    trace();
    int l = 1;

    std::cout << "not use forward: " << std::endl;
    std::cout << "      lvalue pass: ";
    not_use_forward(l);
    std::cout << "      rvalue pass: ";
    not_use_forward(1);

    std::cout << "use forward: " << std::endl;
    std::cout << "      lvalue pass: ";
    use_forward(l);
    std::cout << "      rvalue pass: ";
    use_forward(1);
}

void rvalue_routine() {
    basic_rule();

    move_constructor_example1();

    move_constructor_example2();

    perfect_forwarding();
}
