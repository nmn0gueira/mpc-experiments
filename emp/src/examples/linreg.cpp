/**
 * @file linreg.cpp
 * @brief This prgogram is used for providing a benchmark of a simple linear regression in garbled circuits. Due to the extreme inefficiency of linear regression in garbled 
 * circuits, this program implements a much more limited version of linear regression which provides functionality for only one feature and one label in addition to very basic preprocessing.
 */
#include "../emp/emp-sh2pc/emp-sh2pc/emp-sh2pc.h"
#include "../utils.hpp"

#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

/**
 * Single variable linear regression. Assumes Alice has the feature column and Bob has the labels.
 */
void test_linreg(int party, int input_size) {
	Float *a = new Float[input_size];
	Float *b = new Float[input_size];

	Float secure_input_size = Float(input_size, PUBLIC);
	Float sum_x = Float();	// This value will be the least precise among the sums, even though it still leads to correct result
	Float sum_y = Float();
	Float sum_xy = Float();
	Float sum_x2 = Float();

	utils::initialize_parties<Float>(party, a, b, input_size);

	for (int i = 0; i < input_size; ++i) {
		sum_x = sum_x + a[i];
		sum_y = sum_y + b[i];
		sum_xy = sum_xy + (a[i] * b[i]);
		sum_x2 = sum_x2 + (a[i] * a[i]);
	}
	
	Float beta_1 = (secure_input_size * sum_xy - sum_x * sum_y) / (secure_input_size * sum_x2 - sum_x * sum_x);
	Float beta_0 = (sum_y - beta_1 * sum_x) / secure_input_size;

    cout << "Intercept (beta_0): " << beta_0.reveal<double>() << endl;
	cout << "Slope (beta_1): " << beta_1.reveal<double>() << endl;

	delete[] a;
	delete[] b;
}


int main(int argc, char **argv) {
	if (argc != 5 && argc != 6) {
		cout << "Usage for Alice (server): <program> 1 <port> <input_size> <input_dir>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <input_size> <input_dir>" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	int input_size = atoi(argv[argc - 2]);
	utils::set_directory(argv[argc - 1]);
	
	HighSpeedNetIO * io = new HighSpeedNetIO(ip, port, port + 1);
	auto ctx = setup_semi_honest(io, party);
	ctx->set_batch_size(1024*1024);

	cout << "Party: " << (party == ALICE ? "Alice" : "Bob") << endl;
	cout << "Input size: " << input_size << endl;
	cout << "Input directory: " << utils::get_directory() << endl;

	utils::time_it(test_linreg, party, input_size);

	finalize_semi_honest();

	utils::print_io_stats(*io, party);
	delete io;
	
    return 0;
}
