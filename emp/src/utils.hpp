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

        /* After switching to either sending or receiving (marking the end of one round of communication) a flush is added to the respective channel, meaning
        we can track communication rounds with the help of number of flushes. We still add one more because the last round is not counted properly as the respective channel (and the other one)
        will then flush in the destructor. Regardless this is kind of "makeshifty" without changing the net io code.
        */
        uint64_t comm_rounds = io.schannel->flushes + io.rchannel->flushes + 1; // For now, there are counted like this
        
        // Alice's data sent is the same as Bob's received but for some reason Alice always receives very slightly more data (always ~0.03125 MB on my tests) than Bob sends. 
        // This might have to do with the warning of writing 32769 bytes into a region of size 0 overflowing that I get when compiling which is purely from emp's implementation it seems
        const char* party_str = party == ALICE ? "Alice" : "Bob";
	    cout << "Data sent: " << sent_mb << " MB in " << comm_rounds << " rounds (" << party_str << " only)"<< endl;
        //cout << "Data received: " << recv_mb << "MB" << endl;
        cout << "Global data sent: " << sent_mb + recv_mb << " MB (all parties)" << endl;

    }

}

#endif // UTILS_HPP
