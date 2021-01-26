#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <stdlib.h>
#include <time.h> 
#include "barrier.h"


void worker_thread(Barrier &bar)
{
    srand (time(NULL));
    for (int i=0; i<10; i++) {
        bar.wait();
        std::cout << "a";
        std::cout << bar.wait_and_check(rand() % 2);
    }
}
 
int main()
{
    int num_threads(2);
    Barrier bar(num_threads);
    using namespace std::chrono;  
    std::cout << "in main" << '\n';
    high_resolution_clock::time_point t1 = high_resolution_clock::now(); 

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(worker_thread, std::ref(bar)));
    }

    for (auto &thread: threads) {
        thread.join();
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();   
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1); 
    
    std::cout << "\nback in main. all workers joined" << '\n';
    std::cout << "time elapsed: " << time_span.count() << std::endl;
}
