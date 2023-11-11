#include <cstdlib>
#include <modcpp/base.h>
#include <cassert>
#include <ostream>
#include <source_location>

template<typename... Ts>
void print(const Ts&... args) {
    ((std::cout << args), ...);
    std::cout << std::endl;
}

struct Foo {
    int field_a, field_b;
    friend std::ostream& operator<<(std::ostream& os, const Foo &rhs) {
        os << rhs.field_a << " " << rhs.field_b;
        return os;
    }
};

/**
 * @NodeCapacity: how many objects MemoryNode holds
 */
template<typename T, size_t NodeCapacity = 4, size_t MaxNodeNr = 1024>
class ObjectPool {
private:
    struct MemoryNode {
    private:
        uint8_t *mem;
        uint8_t *current;
    public:
        MemoryNode(const MemoryNode& rhs) = delete;
        MemoryNode& operator=(const MemoryNode& rhs) = delete;
        MemoryNode(MemoryNode&& rhs) : mem(rhs.mem), current(rhs.current) {
            rhs.mem = rhs.current = nullptr;
        }
        MemoryNode& operator=(MemoryNode&& rhs) {
            assert(this != &rhs);
            mem = rhs.mem;
            current = rhs.current;
            rhs.mem = rhs.current = nullptr;
            return *this;
        }
        MemoryNode() : mem(new uint8_t[node_size()]), current(mem) {}
        constexpr size_t get_chunk_size() {
            return sizeof (T) + sizeof (uint8_t *);
        }
        constexpr size_t node_size() {
            return get_chunk_size() * NodeCapacity;
        }
        bool has_free_space() {
            assert((current - mem) % get_chunk_size() == 0);
            return current < mem + node_size();
        }
        uint8_t *allocate_memory() {
            assert(has_free_space());
            auto res = current;
            current += get_chunk_size();
            return res;
        }
        ~MemoryNode() {
            delete[] mem;
            mem = current = nullptr;
        }
    };
    std::vector<MemoryNode> memory_node_list;
    uint8_t **free_list;

    uint8_t *to_object(uint8_t *ptr) {
        return ptr + sizeof (uint8_t *);
    }

    uint8_t **to_rawmemory(T *obj) {
        uint8_t *mem = reinterpret_cast<uint8_t *>(obj);
        mem -= sizeof (uint8_t *);
        return reinterpret_cast<uint8_t **>(mem);
    }

    void create_memory_node() {
        memory_node_list.emplace_back();
    }

    MemoryNode& get_lasted_node() {
        assert(memory_node_list.size());
        return memory_node_list.back();
    }

public:
    ObjectPool() : free_list(nullptr) {
        create_memory_node();
    }

    ObjectPool(const ObjectPool<T, NodeCapacity>& rhs) = delete;
    ObjectPool(ObjectPool<T, NodeCapacity>&& rhs) = delete;
    ObjectPool& operator=(const ObjectPool<T, NodeCapacity>& rhs) = delete;
    ObjectPool& operator=(ObjectPool<T, NodeCapacity>&& rhs) = delete;

    /* []    [a] */
    /* a     b */
    template<typename... Ts>
    T* New(Ts&&... args) {
        if (free_list) [[likely]] {
            uint8_t *prev = *free_list;
            uint8_t *obj = to_object(reinterpret_cast<uint8_t*>(free_list));
            new(obj) T(std::forward<Ts>(args)...);
            free_list = reinterpret_cast<uint8_t**>(prev);
            return reinterpret_cast<T*>(obj);
        }

        if (!get_lasted_node().has_free_space()) [[unlikely]] {
            // no memory on freelist
            create_memory_node();
        }
        auto &node = get_lasted_node();
        assert(node.has_free_space());
        uint8_t *mem = node.allocate_memory();
        uint8_t *obj = to_object(mem);
        new(obj) T(std::forward<Ts>(args)...);
        auto ans = reinterpret_cast<T*>(obj);
        return ans;
    }

    void Release(T* obj) {
        obj->~T();
        uint8_t **mem = to_rawmemory(obj);
        *(mem) = reinterpret_cast<uint8_t*>(free_list);
        free_list = mem;
    }

    // TODO: need to destruct all the object which has been allocated
    ~ObjectPool() {}
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
