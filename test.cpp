#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "barrier.h"


void worker_thread(ThreadGate &bar)
{
    std::cout << "b" << std::flush;
    while (bar.worker_wait()) {
        std::cout << "a" << std::flush;
        std::this_thread::sleep_for (std::chrono::microseconds(100));
    }
}
 
int main()
{
    int num_threads(4);
    ThreadGate bar(num_threads, 10);
    using namespace std::chrono;  
    std::cout << "in main" << std::endl;
    high_resolution_clock::time_point t1 = high_resolution_clock::now(); 

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(worker_thread, std::ref(bar)));
    }

    for (int i = 0; i < 10; ++i) {
        bar.keeper_wait();
        std::cout << std::endl;
    }
    bar.keeper_flush();

    for (auto &thread: threads) {
        thread.join();
    }
    std::cout << std::endl;

    high_resolution_clock::time_point t2 = high_resolution_clock::now();   
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1); 
    
    std::cout << "time elapsed: " << time_span.count() << std::endl;
}
