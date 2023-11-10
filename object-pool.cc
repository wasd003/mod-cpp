#include <modcpp/base.h>
#include <list>
#include <memory>
#include <cassert>

template <typename T>
class DefaultMemoryAllocator {
   public:
    static inline void* Allocate(std::size_t size) {
        long page_num = (size + PageSize() - 1) / PageSize();
        return ::operator new(page_num * PageSize(), ::std::nothrow);
    }
    static inline void Deallocate(void* p, std::size_t size) {
        ::operator delete(p);
    }
    static long PageSize() {
     static long page_size = sysconf(_SC_PAGESIZE);
     return page_size;
 }
};

// this template mainly used to control what behavior occurs when the object
// first allocated, last released, or reused each time.
template <typename T, bool EagerRecycleWhenTrivial = false,
          bool EagerRecycleWhenNotTrivial = true>
struct ObjectPoolTrait {
    static constexpr bool EagerRecycle() {
        return std::is_trivial_v<T> ? EagerRecycleWhenTrivial
                                         : EagerRecycleWhenNotTrivial;
    }

    // Called when the element pointed to by ptr is allocated for the first time
    static void Initialize(T* p) {
        if (!EagerRecycle()) {
            new (p) T();
        }
    }

    // Called when the element pointed to by ptr is freed at the pool
    // destruction time
    static void Cleanup(T* p) {
        if (!EagerRecycle()) {
            p->~T();
        }
    }

    // Called when the element is allocated with the arguments forwarded from
    // ObjectPool::New
    template <typename... Args>
    static void OnAllocate(T* p, Args&&... args) {
        static_assert(sizeof...(Args) == 0 || EagerRecycle(),
                      "only for non-trivial types");
        if (EagerRecycle()) {
            new (p) T(std::forward<Args>(args)...);
        }
    }

    // Called when the element is recycled
    static void OnRecycle(T* p) {
        if (EagerRecycle()) {
            p->~T();
        }
    }
};

// the lazy lifecycle strategy: default-constructed the first time they are
// allocated, and destroyed when the pool itself is destroyed
template <typename T>
using ObjectPoolTraitLazyRecycle = ObjectPoolTrait<T, false, false>;

// eager lifecycle strategy: constructed when they are allocated from the pool
// and destroyed when recycled
template <typename T>
using ObjectPoolTraitEagerRecycle = ObjectPoolTrait<T, true, true>;

namespace _ {

template <typename Pool>
struct ObjectPoolRecycler {
    Pool* pool;

    explicit ObjectPoolRecycler(Pool* pool) : pool(pool) {}

    ObjectPoolRecycler(const ObjectPoolRecycler&) = default;
    ObjectPoolRecycler& operator=(const ObjectPoolRecycler&) = default;

    void operator()(typename Pool::ValueType* p) const { pool->Recycle(p); }
};

}  // namespace _

template <typename T, typename Trait = ObjectPoolTrait<T>,
          class MemoryAllocator = DefaultMemoryAllocator<T>,
          bool CompactLayout = false>
class ObjectPool {
public:
    using ValueType = T;
    using UniquePtr = std::unique_ptr<T, _::ObjectPoolRecycler<ObjectPool>>;

    ObjectPool(std::size_t initial_capacity = 32,
               std::size_t max_node_capacity = 1024 * 1024)
        : size_(0),
          capacity_(initial_capacity),
          max_node_capacity_(max_node_capacity),
          first_deleted_(nullptr),
          nodes_() {
        assert(max_node_capacity >= 1);
        nodes_.emplace_back(initial_capacity);
    }

    ~ObjectPool() {}

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    template <typename... Args>
    UniquePtr New(Args&&... args) {
        auto ptr = first_deleted_;
        if (ptr) {
            first_deleted_ = *reinterpret_cast<T**>(ptr);
            ptr = ToRawPtr(ptr);
        } else {
            assert(!nodes_.empty());
            const auto& last_node = nodes_.back();
            if (last_node.count >= last_node.capacity) {
                AllocateNewNode();
            }
            ptr = nodes_.back().New();
            Trait::Initialize(ptr);
        }

        size_++;
        Trait::OnAllocate(ptr, std::forward<Args>(args)...);
        return UniquePtr(ptr, typename UniquePtr::deleter_type(this));
    }

    void Recycle(T* raw_ptr) {
        Trait::OnRecycle(raw_ptr);
        *reinterpret_cast<T**>(ToPtr(raw_ptr)) = first_deleted_;
        first_deleted_ = raw_ptr;
        size_--;
    }

public:
    // the maximum number of elements held in currently allocated storage
    std::size_t Capacity() const { return capacity_; }

    // the number of elements
    std::size_t Size() const { return size_; }

private:
    static constexpr std::size_t ItemSize() {
        constexpr std::size_t size =
            (sizeof(T) + sizeof(void*) - 1) / sizeof(void*) * sizeof(void*);
        if constexpr (CompactLayout) {
            return size;
        }
        return size + sizeof(void*);
    }

    // |<-  copmact layout(optional)  ->|<-  raw_ptr  ->|
    // ^                                ^
    // |                                |
    // ptr                             raw_ptr
    static constexpr T* ToRawPtr(T* ptr) {
        if constexpr (CompactLayout) {
            return ptr;
        } else {
            return reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) +
                                        sizeof(void*));
        }
    }

    static constexpr T* ToPtr(T* raw_ptr) {
        if constexpr (CompactLayout) {
            return raw_ptr;
        } else {
            return reinterpret_cast<T*>(reinterpret_cast<char*>(raw_ptr) -
                                        sizeof(void*));
        }
    }

    struct Node {
        void* memory;
        std::size_t count;
        std::size_t capacity;

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        Node(std::size_t capacity) : count(0), capacity(capacity) {
            memory = MemoryAllocator::Allocate(capacity * ItemSize());
            if (!memory) {
                /* UNREACHABLE(); */
            }
        }
        ~Node() {
            Free();
            MemoryAllocator::Deallocate(memory, capacity * ItemSize());
        }
        T* New() {
            auto ptr = reinterpret_cast<T*>(static_cast<char*>(memory) +
                                            count * ItemSize());
            count++;
            return ToRawPtr(ptr);
        }
        void Free() {
            if constexpr (Trait::EagerRecycle()) {
                return;
            }
            for (std::size_t cnt = 0; cnt < count; ++cnt) {
                auto ptr = reinterpret_cast<T*>(static_cast<char*>(memory) +
                                                cnt * ItemSize());
                Trait::Cleanup(ToRawPtr(ptr));
            }
        }
    };

    void AllocateNewNode() {
        assert(!nodes_.empty());
        const auto& last_node = nodes_.back();
        auto capacity = std::min(last_node.count * 2, max_node_capacity_);
        nodes_.emplace_back(capacity);

        capacity_ += capacity;
    }

    std::size_t size_;
    std::size_t capacity_;
    std::size_t max_node_capacity_;

    T* first_deleted_;
    std::list<Node> nodes_;
};

void object_pool_routine() {
    ObjectPool<std::string> pool(8);

    std::string str("123456789abcdefg");
    {
        std::vector<ObjectPool<std::string>::UniquePtr> vec;
        for (auto i = 0; i < 16; ++i) {
            vec.emplace_back(pool.New(std::to_string(i) + str));
        }
        assert(pool.Size() == 16);
    }
    assert(pool.Size() == 0);
    assert(pool.Capacity() >= 16);

    struct Foo {
        char* ptr;
        Foo() : ptr(new char) {}
        ~Foo() { delete ptr; }
    };
    // 注意：对于 Lazy 回收模式，由于只会在首次分配和释放时调用构造和析构。
    // 因此，我们不能使用紧凑内存布局，CompactLayout 一定需要设置为 false
    // 否则，对象空间被污染，导致程序异常
    ObjectPool<Foo, ObjectPoolTraitLazyRecycle<Foo>,
        DefaultMemoryAllocator<Foo>, false>
            pool1;
    auto p1 = pool.New();
    { 
        auto p2 = pool.New();
    }
    auto p3 = pool.New();

    assert(pool.Size() == 2);
}
