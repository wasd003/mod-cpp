#include <modcpp/base.h>

class TestClass {
public:
    static std::vector<int> vec;
    static struct StaticConstructor {
        StaticConstructor() {
            for (int i = 0; i < 10; i ++ )
                vec.push_back(i);
        }
    } static_constructor;
};

std::vector<int> TestClass::vec;
TestClass::StaticConstructor TestClass::static_constructor;

void static_member_rountine() {
    while (TestClass::vec.size()) {
        std::cout << TestClass::vec.back() << " ";
        TestClass::vec.pop_back();
    }
}
