#include <memory>
#include <modcpp/base.h>
#include <cassert>
#include <source_location>
#include <set>

template<typename... Ts>
void print(const Ts&... args) {
    ((std::cout << args), ...);
    std::cout << std::endl;
}

static void do_trace(const std::source_location& loc = std::source_location::current()) {
    print(loc.function_name(), ":", loc.line());
}

/**
 * 1. init: use raw pointer to init shared_pointer
 * 2. copy constructor && copy assignment operator: refcnt ++ , points to same memory
 * 3. move constructor && move assignment operator: refcnt remains unchanged.
 * 4. destructor: delete all resource
 * 5. overload ->
 * 6. overload *
 */

template<typename T>
class weak_pointer;

template<typename T>
class shared_pointer {
private:
    struct shared_pointer_metadata {
        int refcnt;
        shared_pointer_metadata() : refcnt(1) {}
    };

    std::set<weak_pointer<T> *> weak_ptr_set;

    shared_pointer_metadata *metadata;
    T *data;

    void decrease_current_refcnt() {
        if (!data) return;
        assert(metadata);
        if ( -- metadata->refcnt == 0) {
            do_trace();
            delete data;
            data = nullptr;
            delete metadata;
            metadata = nullptr;
        }
    }

public:
    shared_pointer() : data(nullptr), metadata(nullptr)  {}

    shared_pointer(T *data) : data(data), metadata(new shared_pointer_metadata()) {}

    shared_pointer(const shared_pointer<T> &rhs) : data(rhs.data), metadata(rhs.metadata) {
        metadata->refcnt ++ ;
    }

    shared_pointer<T>& operator=(const shared_pointer<T> &rhs) {
        decrease_current_refcnt();
        data = rhs.data;
        metadata = rhs.metadata;
        metadata->refcnt ++ ;
        return *this;
    }

    shared_pointer(shared_pointer<T>&& rhs) : data(rhs.data), metadata(rhs.metadata) {
        rhs.data =  nullptr;
        rhs.metadata = nullptr;
    }

    shared_pointer<T>& operator=(shared_pointer<T> && rhs) {
        decrease_current_refcnt();
        data = rhs.data;
        metadata = rhs.metadata;
        rhs.data = nullptr;
        rhs.metadata = nullptr;
        return *this;
    }

    void cancel_subscription(weak_pointer<T>* wp) {
        weak_ptr_set.erase(wp);
    }

    void add_subscription(weak_pointer<T>* wp) {
        weak_ptr_set.insert(wp);
    }

    ~shared_pointer() {
        decrease_current_refcnt();

        while (weak_ptr_set.size()) {
            auto cur = *weak_ptr_set.begin();
            cur->reset();
            weak_ptr_set.erase(cur);
        }
    }

    T* operator->() {
        return data;
    }

    T& operator*() {
        return *data;
    }

    operator bool() {
        return data != nullptr && metadata != nullptr;
    }

    // used for debug and test only
    int get_refcount() {
        if (!metadata) return 0;
        return metadata->refcnt;
    }

};

struct test_struct {
    int field_a, field_b;
    test_struct() : field_a(42), field_b(24) {}
};


/**
 * 1. init: construct from shared_pointer. do not change refcnt
 * 2. copy constructor / copy assignment operator: construct from shared_pointer
 * 2. lock(): get shared_pointer if 
 * 3. expired: test whether shared pointer expired
 *
 */
template<typename T>
class weak_pointer {
private:
    shared_pointer<T> *shared_ptr;

public:
    weak_pointer() : shared_ptr(nullptr) {}

    weak_pointer(shared_pointer<T>& rhs) : shared_ptr(&rhs) {
        shared_ptr->add_subscription(*this);
    }

    shared_pointer<T> lock() {
        if (shared_ptr) return *shared_ptr;
        return shared_pointer<T>();
    }

    weak_pointer<T>& operator=(shared_pointer<T>& rhs) {
        if (shared_ptr != nullptr) shared_ptr->cancel_subscription(this);
        shared_ptr = &rhs;
        shared_ptr->add_subscription(this);
        return *this;
    }

    void reset() {
        shared_ptr = nullptr;
        do_trace();
    }
};

void test_shared_pointer() {
    test_struct *rawpointer = new test_struct();
    shared_pointer<test_struct> sp(rawpointer);
    auto sp2 = sp; // 1. test copy constructor
    auto sp3 = std::move(sp2); // 2. test move constructor
    assert(sp.get_refcount() == sp3.get_refcount() && sp.get_refcount() == 2);
    assert(sp2.get_refcount() == 0);

    test_struct *rawpointer_2 = new test_struct();
    shared_pointer<test_struct> new_sp(rawpointer_2);
    sp2 = new_sp; // 3 test copy assignment operator
    assert(sp2.get_refcount() == new_sp.get_refcount() && sp2.get_refcount() == 2);
    auto sp2_move_pointer = sp;
    sp2_move_pointer = std::move(new_sp); // 4 test move assignment operator
    assert(sp2.get_refcount() == sp2_move_pointer.get_refcount() && sp2.get_refcount() == 2);
    assert(new_sp.get_refcount() == 0);

    // 5. test -> and *
    assert(sp->field_a == 42 && sp->field_b == 24);
    sp->field_a = 10;
    assert(sp->field_a == 10);
    (*sp).field_a = 20;
    assert(sp->field_a == 20);

    print("Pass Shared Pointer Test :)");

}

void test_weak_pointer() {
    weak_pointer<test_struct> wp;
    assert(!wp.lock());
    {
        test_struct *rawpointer = new test_struct();
        shared_pointer<test_struct> sp(rawpointer);
        wp = sp;
        assert(wp.lock());
        assert(wp.lock()->field_a == 42 && wp.lock()->field_b == 24);
    }
    assert(!wp.lock());

    print("Pass Weak Pointer Test :)");
}

void shared_pointer_routine() {
    // test_shared_pointer();
    test_weak_pointer();
}
