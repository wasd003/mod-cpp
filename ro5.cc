#include <cstring>
#include <exception>
#include <modcpp/base.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>

template<typename T>
class RBase {
private:
    T *data;

protected:
    void swap(const RBase& rhs) noexcept {
        swap(data, rhs.data);
    }

public:
    RBase() : data(new T()) {}

    RBase(const RBase& rhs) : data(new T()) {
        memcpy(data, rhs.data, sizeof (T));
    }

    RBase(RBase&& rhs) : data(rhs.data) {
        rhs.data = nullptr;
    }

    RBase& operator=(const RBase& rhs) {
        auto tmp = rhs;
        swap(tmp);
        return *this;
    }

    RBase& operator=(RBase&& rhs) {
        // 之所以临时构造一个对象再swap而不是直接和rhs swap，是因为rhs不能确保一定会在函数结束后被析构
        // 这就有可能导致内存泄漏
        RBase(std::move(rhs)).swap(*this);
        rhs.data = nullptr;
        return *this;
    }

    ~RBase() { 
        delete data;
    }
};

template<typename T>
class RDerived : public RBase<T> {
private:
    T* derived_data;
    int sockfd;

    void swap(const RDerived& rhs) noexcept {
        swap(derived_data, rhs.derived_data);
        swap(sockfd, rhs.sockfd);
    }
public:
    RDerived() : RBase<T>(), derived_data(new T()), sockfd(socket(AF_INET, SOCK_STREAM, 0)) {
        if (sockfd == -1) [[unlikely]] {
            delete derived_data;
            throw std::runtime_error("Error creating socket");
        }
    }

    RDerived(const RDerived& rhs) : RBase<T>(rhs), derived_data(new T()), sockfd(socket(AF_INET, SOCK_STREAM, 0)) {
        if (sockfd == -1) [[unlikely]] {
            delete derived_data;
            throw std::runtime_error("Error creating socket");
        }
        memcpy(derived_data, rhs.derived_data, sizeof (T));
    }

    RDerived(RDerived&& rhs) : RBase<T>(std::move(rhs)), derived_data(rhs.derived_data), sockfd(rhs.sockfd) {
        rhs.derived_data = nullptr;
        rhs.sockfd = 0;
    }

    // FIXME: 这样写依然保证不了原子级别的异常安全，因为可能RBase的operator=成功，但是RDerived部分失败
    RDerived& operator=(const RDerived& rhs) {
        RBase<T>::operator=(rhs);
        auto tmp = rhs;
        swap(tmp);
        return *this;
    }

    // FIXME: 这样写依然保证不了原子级别的异常安全，因为可能RBase的operator=成功，但是RDerived部分失败
    RDerived& operator=(RDerived&& rhs) {
        RBase<T>::operator=(std::move(rhs));
        RDerived(rhs).swap(*this);
        rhs.derived_data = nullptr;
        rhs.sockfd = 0;
        return *this;
    }

    ~RDerived() { 
        close(sockfd);
        delete derived_data;
    }
};

void ro5_routine() {
    try {
        RDerived<int> x;
    } catch (std::exception& e) {
        std::cout << e.what();
    }
    // while (1);
}
