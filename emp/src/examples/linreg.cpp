/**
 * @file linreg.cpp
 * @brief This prgogram is used for providing a benchmark of a simple linear regression in garbled circuits. Due to the extreme inefficiency of linear regression in garbled 
 * circuits, this program implements a much more limited version of linear regression which provides functionality for only one feature and one label in addition to very basic preprocessing.
 */
#include "emp-sh2pc/emp-sh2pc.h"
#include "../utils.hpp"

#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;

/**
 * Single variable linear regression. Assumes Alice has the feature column and Bob has the labels.
 */
void test_linreg(string inputs[], int input_len) {
	Float *a = new Float[input_len];
	Float *b = new Float[input_len];
	Float input_size = Float(input_len, PUBLIC);
	Float sum_x = Float();	// This value will be the least precise among the sums, even though it still leads to correct result
	Float sum_y = Float();
	Float sum_xy = Float();
	Float sum_x2 = Float();

	for (int i = 0; i < input_len; ++i) {
		a[i] = Float(stof(inputs[i]), ALICE);
	}

	for (int i = 0; i < input_len; ++i) {
		b[i] = Float(stof(inputs[i]), BOB);
	}

	for (int i = 0; i < input_len; ++i) {
		sum_x = sum_x + a[i];
		sum_y = sum_y + b[i];
		sum_xy = sum_xy + (a[i] * b[i]);
		sum_x2 = sum_x2 + (a[i] * a[i]);
	}
	
	Float beta_1 = (input_size * sum_xy - sum_x * sum_y) / (input_size * sum_x2 - sum_x * sum_x);
	Float beta_0 = (sum_y - beta_1 * sum_x) / input_size;

    cout << "Intercept (beta_0): " << beta_0.reveal<double>() << endl;
	cout << "Slope (beta_1): " << beta_1.reveal<double>() << endl;

	delete[] a;
	delete[] b;
}


int main(int argc, char **argv) {
	if (argc != 4 && argc != 5) {
		cout << "Usage for Alice (server): <program> 1 <port> <input file>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <input file>" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	char* filename = argv[argc - 1];
	
	HighSpeedNetIO * io = new HighSpeedNetIO(ip, port, port + 1);
	auto ctx = setup_semi_honest(io, party);
	ctx->set_batch_size(1024*1024);
	
	ifstream infile(filename);
	if (!infile.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
		finalize_semi_honest();
		delete io;
		return 1;
    }
	
	vector<string> inputs;
	string line;
	while(getline(infile, line)) {
		inputs.push_back(line);
	}
	infile.close();

	cout << "Number of elements: " << inputs.size() << endl;

	utils::time_it(test_linreg, inputs.data(), inputs.size());

	finalize_semi_honest();

	utils::print_io_stats(*io, party);
	delete io;
	
    return 0;
}
