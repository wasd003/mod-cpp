#include <modcpp/base.h>
#include <list>
#include <string>
#include <array>
#include <optional>
#include <cassert>

template<typename Key, typename Value, typename Hash, auto DefaultSize = 1024>
class HashTable {
private:
    using Node = std::tuple<Key, Value>;

    Hash hash_func;
    std::array<std::list<Node>, DefaultSize> table;
    size_t len;

    size_t get_hash(const Key& key) {
        size_t hash_val = hash_func(key);
        assert(hash_val >= 0 && hash_val < table.size());
        return hash_val;
    }

public:
    HashTable(const Hash& hash_func = Hash()) : hash_func(hash_func), len(0) {}

    Value& operator[](const Key& key) {
        auto hash_val = get_hash(key);
        auto& list = table[hash_val];
        auto iter = std::find_if(list.begin(), list.end(),
                [&key](const Node& node) { return std::get<0>(node) == key; });
        if (iter != list.end()) {
            return std::get<1>(*iter);
        } else {
            // can't find, insert a default value
            len ++ ;
            list.push_back({key, Value()});
            return std::get<1>(list.back());
        }
    }

    void erase(const Key& key) {
        auto hash_val = get_hash(key);
        auto& list = table[hash_val];
        auto iter = std::find_if(list.begin(), list.end(),
                [&key](const Node& node) { return std::get<0>(node) == key; });
        if (iter == list.end()) return;
        list.erase(iter);
    }

    bool count(const Key& key) {
        auto hash_val = get_hash(key);
        auto& list = table[hash_val];
        auto iter = std::find_if(list.begin(), list.end(),
                [&key](const Node& node) { return std::get<0>(node) == key; });
        return iter != list.end();
    }

    size_t size() {
        return len;
    }
};

struct test_struct {
    int field_a, field_b;
};

void hash_table_routine() {
    struct Hash {
        size_t operator()(const std::string& str) {
            size_t res = 0;
            for (auto ch : str) res += ch;
            return res % 1024;
        }
    };
    HashTable<std::string, int, Hash> map;
    map["abc"] = 1;
    assert(map["abc"] == 1);
    assert(map["ab"] == 0);
    assert(map.size() == 2);
}
