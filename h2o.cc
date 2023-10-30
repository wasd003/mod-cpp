#include <condition_variable>
#include <cassert>
#include <mutex>
#include <thread>
#include <atomic>
#include <modcpp/base.h>

std::atomic<int> h_cnt = 0, o_cnt = 0;
constinit std::mutex mtx;
std::condition_variable h_cond, o_cond;


void h2o_generator() {
/**
 *
 * from the perspective of H:
 * case #1: H-cnt < 2:
 *      print anyway, H-cnt ++ 
 * case #2: H-cnt == 2:
 *      case #2.1: H2O has been printed fully
 *              --> clear H and O cnt, notify all sleeping thread
 *      case #2.2: H2O has not been printed fully
 *              --> goto sleep
 */
    int thread_nr = 24;
    std::vector<std::thread> h_thread_list, o_thread_list;
    auto check_fully_print = []() {
        return h_cnt == 2 && o_cnt == 1;
    };
    auto reset_state = []() {
        o_cnt = h_cnt = 0;
        h_cond.notify_all();
        o_cond.notify_all();
    };
    for (int i = 0; i < thread_nr; i ++ ) {
        h_thread_list.emplace_back([&check_fully_print, &reset_state](int tid) {
            for (;;) {
                std::unique_lock<std::mutex> lk(mtx);
                if (check_fully_print()) {
                    reset_state();
                } else {
                    h_cond.wait(lk, []() {
                        return h_cnt < 2;
                    });
                    assert(h_cnt < 2);
                    std::cout << "H";
                    h_cnt ++ ;
                    assert(h_cnt <= 2);
                }
            }
        }, i);
    }

    for (int i = 0; i < thread_nr; i ++ ) {
        o_thread_list.emplace_back([&check_fully_print, &reset_state](int tid) {
            for (;;) {
                std::unique_lock<std::mutex> lk(mtx);
                if (check_fully_print()) {
                    reset_state();
                } else {
                    o_cond.wait(lk, []() { return o_cnt < 1; });
                    assert(o_cnt < 1);
                    std::cout << "O";
                    o_cnt ++ ;
                    assert(o_cnt == 1);
                }
            }
        }, i + thread_nr);
    }
    for (int i = 0; i < thread_nr; i ++ ) {
        h_thread_list[i].join();
        o_thread_list[i].join();
    }
}
