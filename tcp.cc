#include <exception>
#include <modcpp/base.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <cassert>
#include <unistd.h>
#include <optional>
#include <queue>
#include <unordered_set>
/**
 * sender: 
 * @api: provide app layer send syscall interface
 *
 *      1. if data can be fit into sender buffer:
 *          send data && put data into buffer
 *      2. othervise, refuse 
 *
 * @api: timer expired: use a thread to inc atomic counter
 *      1. reset timer
 *      2. send all pkts int current window
 *
 * @api: receive ack
 *      1. reset timer
 *      2. move send_base
 *
 *  2 thread: 
 *      1) timer thread: inc timer
 *      2) handler thread: handle 2 events: a) timer expire b) receive ack
 */

struct ip_pkt {
    uint8_t data;
    uint64_t seq;
};

std::mutex ack_queue_mtx;
std::queue<ip_pkt> ack_queue;

std::mutex pkt_queue_mtx;
std::queue<ip_pkt> pkt_queue;

template<auto SendWindow = 1024>
class TCP_Sender;

template<auto ReceiveWindow = 1024>
class TCP_Receiver;

template<auto ReceiveWindow>
class TCP_Receiver {
private:
    std::condition_variable recv_cond;
    std::condition_variable *send_cond;
    std::thread recver;
    uint64_t recv_base;
    ip_pkt tcp_buf[ReceiveWindow];
    std::queue<ip_pkt> app_buf;
    std::mutex app_buf_mtx;
    std::unordered_set<uint64_t> ready_seq;
    template<auto SendWindow>
    friend class TCP_Sender;

    void send_ack(const ip_pkt& pkt) {
        if (send_cond == nullptr) throw std::runtime_error("connection loss");
        std::unique_lock<std::mutex> lk(ack_queue_mtx);
        ack_queue.push(pkt);
        send_cond->notify_all();
    }

    void do_recv() {
        std::unique_lock<std::mutex> lk(pkt_queue_mtx);
        recv_cond.wait(lk, []() { return pkt_queue.size(); });
        while (pkt_queue.size()) {
            // [recv_base, upper)
            uint64_t upper = recv_base + ReceiveWindow;
            auto pkt = pkt_queue.front(); pkt_queue.pop();
            auto seq = pkt.seq;
            if (seq >= upper) continue;
            if (seq >= recv_base) {
                tcp_buf[seq % ReceiveWindow] = pkt;
                ready_seq.insert(seq);
            }
        }
        // update recv_base and do acculate ack
        {
            std::unique_lock<std::mutex> lk(app_buf_mtx);
            while (ready_seq.count(recv_base)) {
                app_buf.push(tcp_buf[recv_base % ReceiveWindow]);
                recv_base ++ ;
            }
        }
        send_ack({0, recv_base});
    }

public:
    TCP_Receiver() : send_cond(nullptr), recv_base(0) {
        recver = std::thread(do_recv);
    }

    std::optional<ip_pkt> tcp_recv() {
        std::unique_lock<std::mutex> lk(app_buf_mtx);
        if (app_buf.empty()) return std::nullopt;
        auto res = app_buf.front(); app_buf.pop();
        return res;
    }
};

template<auto SendWindow>
class TCP_Sender {
private:
    constexpr static int TimeOutLimit = 5;
    std::thread timer, handler;
    ip_pkt buf[SendWindow];
    uint64_t send_base, current_seq;
    std::atomic<int> tcp_counter;
    std::mutex handler_mtx;
    std::condition_variable handler_cond;
    bool terminate_timer_thread;
    bool terminate_handler_thread;
    std::condition_variable *recv_cond;

    template<auto ReceiveWindow>
    friend class TCP_Receiver;

    void reset_timer() {
        tcp_counter.store(0);
    }

    void ip_send(const ip_pkt& pkt) {
        std::unique_lock<std::mutex> lk(pkt_queue_mtx);
        pkt_queue.push(pkt);
        if (recv_cond == nullptr) throw std::runtime_error("connection loss");
        recv_cond->notify_all();
    }

    void handle_ack() {
        std::unique_lock<std::mutex> lk(ack_queue_mtx);
        while (ack_queue.size()) {
            auto [_, seq] = ack_queue.front(); ack_queue.pop();
            send_base = std::max(send_base, seq);
        }
        reset_timer();
    }

    void handle_timeout() {
        for (int i = send_base; i < current_seq; i ++ ) {
            ip_send(buf[i % SendWindow]);
        }
        reset_timer();
    }

    void handler_fn() {
        std::unique_lock<std::mutex> lk(handler_mtx);
        handler_cond.wait(lk, [this]() {
            return tcp_counter.load() >= TimeOutLimit || ack_queue.size();
        });
        if (ack_queue.size()) {
            handle_ack();
        }
        if (tcp_counter.load() >= TimeOutLimit) {
            handle_timeout();
        }
    }

public:
    TCP_Sender() :
        send_base(0), current_seq(0),
        tcp_counter(0), terminate_timer_thread(false),
        terminate_handler_thread(false), recv_cond(nullptr)
    {
        timer = std::thread([this]() {
            for (;!terminate_timer_thread;) {
                sleep(1);
                tcp_counter ++ ;
                handler_cond.notify_all();
            }
        });
        handler = std::thread(handler_fn);
    }

    void tcp_send(uint8_t data) {
        if (recv_cond == nullptr) throw std::runtime_error("connection loss");
        int cur_len = current_seq - send_base;
        if (cur_len >= SendWindow) throw std::runtime_error("sender buffer is full");
        buf[current_seq % SendWindow] = {data, current_seq};
        ip_send(buf[current_seq % SendWindow]);
        current_seq ++ ;
        recv_cond->notify_all();
        reset_timer();
    }

    template<auto ReceiveWindow>
    void tcp_connect(const TCP_Receiver<ReceiveWindow> &recv) {
        recv_cond = &recv.recv_cond;
        recv.send_cond = &handler_cond;
    }

    ~TCP_Sender() {
        terminate_timer_thread = terminate_handler_thread = true;
        timer.join();
        handler.join();
    }
};
