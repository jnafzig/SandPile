#ifndef BAR_H
#define BAR_H

#include <mutex>
#include <condition_variable>
 
class ThreadGate {
private:
    std::mutex mutex;
    std::condition_variable pool_cv;
    std::condition_variable keeper_cv;
    bool iterations_finished;
    bool not_done;
    std::size_t iterations;
    std::size_t iteration_cap;
    std::size_t counter;
    std::size_t thread_count;
public:
    explicit ThreadGate(std::size_t num_threads, std::size_t iteration_cap) : 
        not_done(true),
        iterations_finished(false),
        iterations(0),
        iteration_cap(iteration_cap),
        counter(num_threads), 
        thread_count(num_threads) { }
    bool worker_wait()
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (iterations >= iteration_cap) {
            if (--counter==0) {
               //std::cout << "l" << std::flush;
               iterations_finished = true;
               keeper_cv.notify_one();
            }
            //std::cout << "w" << std::flush;
            pool_cv.wait(lock, [this] { return iterations < iteration_cap; });
            //std::cout << "f" << std::flush;
            counter++;
        }
        if (not_done) {
            iterations++;
        }
        return not_done;
    }    
    void keeper_flush()
    {
        not_done = false;
        iterations = 0;
        pool_cv.notify_all();
    } 
    void keeper_wait()
    {
        std::unique_lock<std::mutex> lock(mutex);
        //std::cout << "k" << std::flush;
        pool_cv.notify_all();
        keeper_cv.wait(lock, [this] { return iterations_finished; });
        iterations_finished = false;
        iterations = 0;
    } 
};

#endif
