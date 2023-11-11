#include <cstddef>
#include <cstdint>
#include <ios>
#include <modcpp/base.h>
#include <set>
#include <map>
#include <unordered_set>
#include <cassert>

template<auto NodeSize = 4096>
class MemoryAllocator {
private:
    struct MemoryNode {
        uint8_t *mem;

        MemoryNode() : mem(new uint8_t [NodeSize]) {}

        MemoryNode(const MemoryNode& rhs) = delete;

        MemoryNode& operator=(const MemoryNode& rhs) = delete;

        MemoryNode(MemoryNode&& rhs) : mem(rhs.mem) {
            rhs.mem = nullptr;
        }

        MemoryNode& operator=(MemoryNode&& rhs) {
            assert(this != &rhs);
            mem = rhs.mem;
            rhs.mem = nullptr;
            return *this;
        }

        ~MemoryNode() {
            delete[] mem;
        }
    };

    using MemoryRegion = std::tuple<uint64_t, uint64_t, uint64_t>; // len, left, right

    std::vector<MemoryNode> memory_node_list;

    std::set<MemoryRegion> free_memory_region;

    std::unordered_map<uint64_t, uint64_t> left2right, right2left;

    std::map<uint64_t, uint64_t> addr2len;

    void create_memory_node() {
        memory_node_list.emplace_back();
        auto& node = memory_node_list.back();
        uint64_t mem = reinterpret_cast<uint64_t>(node.mem);
        insert_memory_region(mem, mem + NodeSize);
    }

    void delete_memory_region(uint64_t left, uint64_t right) {
        MemoryRegion mr {right - left, left, right};
        assert(free_memory_region.count(mr));
        free_memory_region.erase(mr);

        assert(right2left.count(right));
        right2left.erase(right);

        assert(left2right.count(left));
        left2right.erase(left);
    }

    void insert_memory_region(uint64_t left, uint64_t right) {
        MemoryRegion mr {right - left, left, right};
        assert(!free_memory_region.count(mr));
        free_memory_region.insert(mr);

        assert(!right2left.count(right));
        right2left[right] = left;

        assert(!left2right.count(left));
        left2right[left] = right;
    }
    
public:
    MemoryAllocator() {
        create_memory_node();
    }

    uint8_t *allocate(size_t alloc_len) {
        if (alloc_len > NodeSize) [[unlikely]]
            throw std::runtime_error("can't allocate memory larger than NodeSize");
        auto iter = free_memory_region.lower_bound({alloc_len, 0, 0});
        if (iter == free_memory_region.end())
            create_memory_node();
        iter = free_memory_region.lower_bound({alloc_len, 0, 0});
        assert(iter != free_memory_region.end());
        const auto [mr_len, left, right] = *iter;
        size_t remain_len = alloc_len - mr_len;
        assert(remain_len >= 0);
        uint8_t *res = reinterpret_cast<uint8_t *>(left);
        
        // delete stale metadata
        delete_memory_region(left, right);
        
        // update metadata
        if (remain_len) {
            auto new_left = left + alloc_len;
            insert_memory_region(new_left, right);
        }
        addr2len[left] = alloc_len;
        return res;
    }

    void release(uint8_t *mem_ptr) {
        uint64_t mem = reinterpret_cast<uint64_t>(mem_ptr);
        assert(addr2len.count(mem));
        uint64_t left = mem, right = mem + addr2len[mem];
        addr2len.erase(mem);

        if (right2left.count(left)) {
            auto old_right = left, old_left = right2left[old_right];
            // merge with prev region
            left = old_left;
            // delete stale metadata
            delete_memory_region(old_left, old_right);
        }

        if (left2right.count(right)) {
            auto old_left = right, old_right = left2right[old_left];
            // merge with next region
            right = old_right;
            // delete stale metadata
            delete_memory_region(old_left, old_right);
        }

        insert_memory_region(left, right);
    }
};

void memory_allocator_routine() {
    std::vector<uint8_t *> vec, vec2;
    MemoryAllocator alloc;
    for (int i = 8; i < 1024; i += 8) {
        auto ptr = alloc.allocate(i);
        *(uint64_t*)ptr = i;
        vec.push_back(ptr);
    }
    for (int i = 0; i < vec.size(); i ++ ) {
        alloc.release(vec[i]);
        vec[i] = nullptr;
    }
    for (int i = 8; i < 1024; i += 8) {
        auto ptr = alloc.allocate(i);
        *(uint64_t*)ptr = i;
        vec2.push_back(ptr);
    }

    for (int i = 0; i < vec.size(); i ++ ) {
        alloc.release(vec2[i]);
        vec2[i] = nullptr;
    }
    std::cout << "Pass All Test :)\n";
}
