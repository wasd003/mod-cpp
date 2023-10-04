#include <modcpp/base.h>
#include <thread>
#include <future>
#include <queue>


void promise_example() {
    std::promise<int> pro;
    auto fut = pro.get_future();

    std::thread producer([&pro]() {
        pro.set_value(1);
    });

    std::thread consumer([&fut]() {
            try {
                auto val = fut.get();
                std::cout << val << std::endl;
                for (;;);
            } catch(const std::exception &e) {
                std::cout << "Consumer caught exception: " << e.what() << std::endl;
            }
        }
    );

    producer.join();
    consumer.join();
}

void packaged_task_example() {
    std::mutex mtx;
    /**
     * NOTE: use deque instead of queue!!
     */
    std::deque<std::packaged_task<int(void)>> task_queue;
    auto task_func = []() {
        return 42;
    };

    std::thread worker([&mtx, &task_queue]() {
        for (;;) {
            std::unique_lock lk(mtx);
            if (task_queue.empty()) continue;
            auto tsk = std::move(task_queue.front());
            task_queue.pop_front();
            lk.unlock();
            tsk();
        }
    });

    try {
        for (;;) {
            std::packaged_task<int(void)> task(task_func);
            auto fut = task.get_future();
            std::unique_lock lk(mtx);
            task_queue.push_back(std::move(task));
            lk.unlock();
            auto ret = fut.get();
            std::cout << "ret:" << ret << std::endl;
        }
    } catch(std::exception &e) {
        std::cout << "caught exception: " << e.what() << std::endl;
    }
}

void future_routine() {
    /* promise_example(); */

    packaged_task_example();
}
