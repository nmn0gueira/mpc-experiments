#ifndef UTILS_HPP
#define UTILS_HPP

#include "emp-sh2pc/emp-sh2pc.h"
#include <chrono>
#include <iostream>
#include <utility>
using namespace std;

const int BITSIZE = 32;

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

    const std::string& get_directory() {
        static std::string directory;
        return directory;
    }

    void set_directory(const std::string& dir) {
        static bool is_set = false;
        if (is_set) {
            throw std::runtime_error("Directory already set");
        }
        const_cast<std::string&>(get_directory()) = dir;
        is_set = true;
    }


    ifstream get_input_file(char col) {
        string file_path = get_directory() + "/" + col + ".dat";
        ifstream infile(file_path);
        if (!infile.is_open()) {
            cerr << "Failed to open file: " << file_path << endl;
            exit(1);
        }
        return infile;
    }

    void initialize_values(int party, int other_party, Integer * party_values, Integer * other_party_values, int input_size, ifstream & infile, string & line) {
        for (int i = 0; i < input_size; ++i) {
            getline(infile, line);
            party_values[i] = Integer(BITSIZE, stoi(line), party);
        }
        for (int i = 0; i < input_size; ++i) {
            other_party_values[i] = Integer(BITSIZE, 0, other_party);
        }
    }

    void initialize_values(int party, int other_party, Float * party_values, Float * other_party_values, int input_size, ifstream & infile, string & line) {
        for (int i = 0; i < input_size; ++i) {
            getline(infile, line);
            party_values[i] = Float(stof(line), party);
        }
        for (int i = 0; i < input_size; ++i) {
            other_party_values[i] = Float(0, other_party);
        }
    }

    template <typename T>
    void initialize_parties(int party, T *alice, T *bob, int input_size) {
        T *current_party_ptr;
        T *other_party_ptr;
        int other_party;

        if (party == ALICE) {
            current_party_ptr = alice;
            other_party_ptr = bob;
            other_party = BOB;
        } else {
            current_party_ptr = bob;
            other_party_ptr = alice;
            other_party = ALICE;
        }

        ifstream infile = get_input_file('0');
        string line;

        initialize_values(party, other_party, current_party_ptr, other_party_ptr, input_size, infile, line);
    }
}

#endif // UTILS_HPP
