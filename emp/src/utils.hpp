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
        
        const char* party_str = party == ALICE ? "Alice" : "Bob";
	    cout << "Data sent (" << party_str << "): " << sent_mb << " MB" << endl;
        cout << "Data sent (global): " << sent_mb + recv_mb << " MB" << endl;
    }

}

#endif // TIMING_UTILS_HPP
