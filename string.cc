#include <modcpp/base.h>
#include <string.h>
#include <cassert>

template<typename... Ts>
void print(const Ts&... args) {
    ((std::cout << args), ...);
    std::cout << std::endl;
}

class string {
private:
    char *data;
    size_t str_len;

    void create_and_copy(const string& rhs) {
        int n = rhs.size();
        data = new char[n];
        str_len = n;
        for (int i = 0; i < n; i ++ )
            data[i] = rhs[i];
    }

    string(const char *str, int n) {
        int len = strlen(str);
        assert(len >= n);
        data = new char[n];
        str_len = n;
        for (int i = 0; i < n; i ++ )
            data[i] = str[i];
    }

public:
    string(const char *str) {
        int n = strlen(str);
        data = new char[n];
        str_len = n;
        for (int i = 0; i < n; i ++ )
            data[i] = str[i];
    }


    string(const string& rhs) {
        create_and_copy(rhs);
    }

    string& operator=(const string& rhs) {
        if (this == &rhs) return *this;
        if (data) {
            delete[] data;
            data = nullptr;
            str_len = 0;
        }
        create_and_copy(rhs);
        return *this;
    }

    string(string&& rhs) noexcept: data(rhs.data), str_len(rhs.size()) {
        rhs.data = nullptr;
        rhs.str_len = 0;
    }

    string& operator=(string&& rhs) noexcept {
        assert(this != &rhs);
        if (data) {
            delete[] data;
            data = nullptr;
            str_len = 0;
        }
        data = rhs.data;
        rhs.data = nullptr;
        rhs.str_len = 0;
        return *this;
    }

    ~string() {
        delete[] data;
        data = nullptr;
        str_len = 0;
    }

    char& operator[](int idx) const {
        size_t n = size();
        if (idx < 0 || idx >= n) throw std::runtime_error("index outof bound");
        return data[idx];
    }

    bool operator==(const string& rhs) const {
        if (size() != rhs.size()) return false;
        int n = size();
        for (int i = 0; i < n; i ++ )
            if (data[i] != rhs[i])
                return false;
        return true;
    }

    string substr(int idx, int len) {
        int n = size();
        if (idx < 0 || idx >= n) throw std::runtime_error("substr outof bound");
        if (len < 0) throw std::runtime_error("substr negative len");
        if (!len) [[unlikely]] return string("");
        int left = idx, right = std::min(n - 1, idx + len - 1);
        return string(data + idx, right - left + 1);
    }

    size_t size() const {
        return str_len;
    }

    friend std::ostream& operator<<(std::ostream& os, const string& rhs) {
        int n = rhs.size();
        for (int i = 0; i < n; i ++ ) {
            os << rhs.data[i];
        }
        return os;
    }
};

void string_routine() {
    string s1 = "abc";
    string s2 = "bc";
    string s3 = s1.substr(1, 1000);
    assert(s2 == s3);
    s1[1] = '\0';
    assert(s1.size() == 3);
    std::cout << s1 << std::endl;

    print("All String Tests Passed :)");
}
