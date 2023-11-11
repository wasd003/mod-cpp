#include <modcpp/base.h>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <modcpp/base.h>
#include <set>
#include <map>
#include <unordered_set>
#include <cassert>
#include <sstream>

using namespace std;

using MemoryRegion = std::tuple<uint64_t, uint64_t, uint64_t>; // len, left, right
std::set<MemoryRegion> free_memory_region;

uint64_t hexStringToUint64(const std::string& hexString) {
    std::stringstream ss;
    ss << std::hex << hexString;
    uint64_t result;
    ss >> result;
    return result;
}

void test_routine() {
    string op;
    size_t len;
    string left_str, right_str;
    while (cin >> op >> len >> left_str >> right_str) {
        auto left = hexStringToUint64(left_str),
             right = hexStringToUint64(right_str);
        // cout << op << " " << len << " " << left << " " << right << endl;
        if (op == "insert") {
            free_memory_region.insert({right - left, left, right});
        } else if (op == "delete") {
            free_memory_region.erase({right - left, left, right});
        } else if (op == "count") {
            assert(!free_memory_region.count({right - left, left, right}));
        }
    }
}
