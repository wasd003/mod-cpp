#include <cstdlib>
#include <modcpp/base.h>
#include <cassert>
#include <ostream>
#include <source_location>
#include <list>

template<typename... Ts>
void print(const Ts&... args) {
    ((std::cout << args), ...);
    std::cout << std::endl;
}

static void do_trace(const std::source_location& loc = std::source_location::current()) {
    print(loc.function_name(), ":", loc.line());
}

struct Foo {
    int field_a, field_b;
    friend std::ostream& operator<<(std::ostream& os, const Foo &rhs) {
        os << rhs.field_a << " " << rhs.field_b;
        return os;
    }
    Foo(int fa, int fb) : field_a(fa), field_b(fb) {}
    Foo() = default;
};

/**
 * NodeCapacity: how many objs does node hold
 */
template<typename T, auto NodeCapacity = 4>
class ObjectPool {
private:
    struct MemoryNode {
        T* mem;
        int cur_free_idx;
        MemoryNode() : mem(new T[NodeCapacity]), cur_free_idx(0) {}
        MemoryNode(const MemoryNode& rhs) = delete;
        MemoryNode operator=(const MemoryNode& rhs) = delete;
        MemoryNode& operator=(MemoryNode&& rhs) = delete;
        MemoryNode(MemoryNode&& rhs) : mem(rhs.mem), cur_free_idx(rhs.cur_free_idx) {
            rhs.mem = nullptr;
            rhs.cur_free_idx = 0;
        }
        bool full() {
            return cur_free_idx >= NodeCapacity;
        }
        T *allocate() {
            assert(!full());
            auto ans = &mem[cur_free_idx];
            cur_free_idx ++ ;
            return ans;
        }
        ~MemoryNode() {
            do_trace();
            delete[] mem;
        }
    };

    /**
     * allocate raw memory
     */
    T *allocate() {
        // 1. try to allocated from free list
        if (freelist.size()) {
            auto res = freelist.front();
            freelist.pop_front();
            return res;
        }

        // 2. if free list is empty, try to allocate from latest node
        assert(mem_node_list.size());
        if (mem_node_list.back().full()) {
            mem_node_list.emplace_back();
        }
        return mem_node_list.back().allocate();
    }

    std::vector<MemoryNode> mem_node_list;

    std::list<T*> freelist;

public:
    ObjectPool() {
        mem_node_list.emplace_back();
    }

    template<typename... Args>
    T *New(Args&&... args) {
        T *raw_mem = allocate();
        new (raw_mem) T(std::forward<Args>(args)...);
        return raw_mem;
    }

    void Release(T *obj) {
        obj->~T();
        freelist.push_back(obj);
    }

};


void object_pool_routine() {
    ObjectPool<Foo> pool;

    std::vector<Foo*> vec;
    for (auto i = 0; i < 16; ++i) {
        vec.push_back(pool.New(i, i));
        std:: cout << *vec[i] << std::endl;
    }
    for (int i = 0; i < 16; i ++ ) {
        pool.Release(vec[i]);
        vec[i] = nullptr;
    }

    auto p1 = pool.New();
    auto p2 = pool.New();
    pool.Release(p1);
    pool.Release(p2);

    print("Pass All Test :)");
}
