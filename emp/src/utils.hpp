#ifndef UTILS_HPP
#define UTILS_HPP

#include "emp-sh2pc/emp-sh2pc.h"
#include <chrono>
#include <iostream>
#include <utility>
using namespace std;


namespace utils {

    /**
     * @brief Measures the execution time of a function. Default arguments will need to be passed explicitly.
     */
    template <typename Func, typename... Args>
    void time_it(Func&& func, Args&&... args) {
        using namespace std::chrono;

        auto start = high_resolution_clock::now();

        // Call the function directly (C++11 compatible)
        func(std::forward<Args>(args)...);

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);

        std::cout << "---------------------------------------------" << std::endl;
        std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
    }

    /**
     * @brief Prints the information about communication
     */
    void print_io_stats(const HighSpeedNetIO& io, int party) {
        double sent_mb = (double) io.schannel->counter / (1024 * 1024.0);
        double recv_mb = (double) io.rchannel->counter / (1024 * 1024.0);
        
        // Alice's data sent is the same as Bob's received but for some reason Alice always receives very slightly more data (always ~0.03125 MB on my tests) than Bob sends. 
        // This might have to do with the warning of writing 32769 bytes into a region of size 0 overflowing that I get when compiling which is purely from emp's implementation it seems
        const char* party_str = party == ALICE ? "Alice" : "Bob";
	    cout << "Data sent: " << sent_mb << " MB (" << party_str << " only)"<< endl;
        //cout << "Data received: " << recv_mb << "MB" << endl;
        cout << "Global data sent: " << sent_mb + recv_mb << " MB (all parties)" << endl;
        // Use number of flushes for benchmarking communication rounds? Need to look further into this

    }

}

#endif // UTILS_HPP
