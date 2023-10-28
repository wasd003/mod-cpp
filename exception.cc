#include <exception>
#include <pthread.h>
#include <source_location>
#include <modcpp/base.h>
#include <stdexcept>
#include <string>

class custom_exception : public std::runtime_error {
public:
    custom_exception(const std::string& str) : std::runtime_error(str) {}
};

void f4() { 
    const auto& loc = std::source_location::current();
    throw custom_exception(
            std::string(loc.function_name()) + ":" + std::to_string(loc.line()));
}
void f3() { f4(); }
void f2() { f3(); }
void f1() { f2(); }

void basic_usage() {
    try {
        f1();
    } catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
    } catch (...) {
        std::cout << "unable to handle exception" << std::endl;
        std::terminate();
    }
}

void exception_handler() {
    try {
        throw;
    } 
    catch (const custom_exception& err) {
        std::cout << "custom_exception" << std::endl;
    } 
    catch (const std::runtime_error& err) {
        std::cout << "runtime_error" << std::endl;
    } 
}

void exception_handler_example() {
    try {
        f1();
    } catch (...) {
        exception_handler();
    }
}

class base_exception : public std::runtime_error {
public:
    base_exception(const std::string& str) : std::runtime_error(str) {}

    virtual void raise() {
        throw *this;
    }
};

class derived_exception : public base_exception {
public:
    derived_exception(const std::string& str) : base_exception(str) {}

    void raise() override {
        throw *this;
    }
};

void throw_exception_polymorphically() {
    try {
        derived_exception derived_err = derived_exception("derived exception");
        base_exception& base_err = derived_err;
        base_err.raise();
    }
    catch (const derived_exception& err) {
        std::cout << "deirived exception" << std::endl;
    }
    catch (const base_exception& err) {
        std::cout << "base exception" << std::endl;
    }
}

void exception_routine() {
    basic_usage();

    exception_handler_example();

    throw_exception_polymorphically();
}
