#ifndef BAR_H
#define BAR_H

#include <mutex>
#include <condition_variable>
 
class Barrier {
private:
    std::mutex mutex;
    std::condition_variable cv;
    bool all_done;
    bool return_val;
    std::size_t gen;
    std::size_t counter;
    std::size_t thread_count;
public:
    explicit Barrier(std::size_t num_threads) : 
        counter(num_threads),
        gen(0),
	    all_done(true),
	    return_val(true),
        thread_count(num_threads) { }
    void wait()
    {
        std::unique_lock<std::mutex> lock(mutex);
	    auto current_gen = gen;
        if (--counter == 0) {
            gen++;
	        counter = thread_count;
            cv.notify_all();
        } else {
            cv.wait(lock, [this, current_gen] { return current_gen != gen; });
        }
    }    
    bool wait_and_check(bool done)
    {
        std::unique_lock<std::mutex> lock(mutex);
	    if (not done) {all_done=false;}
	    auto current_gen = gen;
        if (--counter == 0) {
            gen++;
	        counter = thread_count;
            return_val = all_done;
            all_done = true;
            cv.notify_all();
        } else {
            cv.wait(lock, [this, current_gen] { return current_gen != gen; });
        }
	    return return_val;
    }    
};

#endif
