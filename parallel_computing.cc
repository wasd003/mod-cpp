#include <condition_variable>
#include <mutex>
#include <queue>
#include <future>
#include <thread>
#include <modcpp/base.h>


constexpr static int N = 1000, worker_nr = 12;

struct task {
    std::function<void(int, int, int)> tsk;
    int row;
    int start_col, end_col;
};

struct result {
    int row;
    int start_col, end_col;
};

constinit std::mutex worker_mtx_list[worker_nr];
std::condition_variable worker_cond_list[worker_nr];
std::deque<task> work_queue[worker_nr];

constinit std::mutex scheduler_mtx;
std::condition_variable scheduler_cond;
std::deque<result> complete_queue;

std::array<std::array<int, N>, N> f;

volatile bool work_all_done = false;

std::vector<std::thread> spawn_worker_thread() {
    std::vector<std::thread> worker_list;
    for (int i = 0; i < worker_nr; i ++ ) {
        worker_list.emplace_back([](int thread_id) {
            for (;!work_all_done;) {
                std::unique_lock<std::mutex> lk(worker_mtx_list[thread_id]);
                worker_cond_list[thread_id].wait(lk, [thread_id]() {
                    return work_queue[thread_id].size();
                });
                auto& my_work_queue = work_queue[thread_id];
                auto& [tsk, row, start_col, end_col] = my_work_queue.front();
                my_work_queue.pop_front();
                tsk(row, start_col, end_col);
                std::unique_lock scheduler_lock(scheduler_mtx);
                complete_queue.emplace_back(row, start_col, end_col);
                scheduler_cond.notify_all();
            }
        }, i);
    }
    return worker_list;
}

void parallel_computing_routine() {
    f[0].fill(1);
    std::thread scheduler([]() {
        auto worker_list = spawn_worker_thread();
        auto calc_func = [](int row, int start_col, int end_col) {
            for (int col = start_col; col < end_col; col ++ ) {
                f[row][col] = (col >= 1 ? f[row - 1][col - 1] : 0) + f[row - 1][col];
            }
        };
        for (;!work_all_done;) {
            std::unique_lock<std::mutex> scheduler_lock(scheduler_mtx);
            scheduler_cond.wait([]() {
                return complete_queue.size();
            });
            while (!complete_queue.empty()) {
                auto [row, start_col, end_col] = complete_queue.front();
                complete_queue.pop_front();
                update_compute_process(row, start_col, end_col);
            }
        }
    });

    scheduler.join();
    for (int i = 0; i < N; i ++ ) {
        for (int j = 0; j < N; j ++ ) {
            std::cout << f[i][j] << " ";
        }
        std::cout << std::endl;
    }
}
