#ifndef TIMING_UTILS_HPP
#define TIMING_UTILS_HPP

#include <chrono>
#include <iostream>
#include <utility>

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

}

#endif // TIMING_UTILS_HPP
