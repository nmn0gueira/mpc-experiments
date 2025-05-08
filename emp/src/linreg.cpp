#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;

/**
 * Standard scaler function for scaling the input data.
 */
void standard_scaler(Float * a, int size) {
	Float mean = Float();
	Float std = Float();
	Float count(size, PUBLIC);
	for (int i = 0; i < size; ++i)
		mean = mean + a[i];

	mean = mean / count;

	for (int i = 0; i < size; ++i)
		std = std + (a[i] - mean).sqr();

	std = (std/count).sqrt();

	for (int i = 0; i < size; ++i)
		a[i] = (a[i] - mean) / std;

	cout << "Mean: " << mean.reveal<double>() << endl;
	cout << "Std: " << std.reveal<double>() << endl;
}


/**
 * Single variable linear regression. Assumes Alice has the feature column and Bob has the labels.
 */
void test_linreg(int party, string inputs[], int input_len, bool scale=true) {
	Float *a = new Float[input_len];
	Float *b = new Float[input_len];
	Float input_size = Float(input_len, PUBLIC);
	Float sum_x = Float();
	Float sum_y = Float();
	Float sum_xy = Float();
	Float sum_x2 = Float();

	for (int i = 0; i < input_len; ++i) {
		a[i] = Float(stoi(inputs[i]), ALICE);
		b[i] = Float(stoi(inputs[i]), BOB);
	}

	if (scale)
		standard_scaler(a, input_len);


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
	
	NetIO * io = new NetIO(ip, port);
	auto ctx = setup_semi_honest(io, party);
	ctx->set_batch_size(1024*1024);	// I assume this makes the process faster when working with floats
	
	ifstream infile(filename);
	if (!infile.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
		delete io;
		return 1;
    }
	
	vector<string> inputs;
	string line;
	while(getline(infile, line)) {
		inputs.push_back(line);
	}
	infile.close();

	auto start = chrono::high_resolution_clock::now();
	test_linreg(party, inputs.data(), inputs.size());
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
	cout << "Execution time of linreg with " << inputs.size() << " elements: " << duration << " ms" << endl;
	
	delete io;
    return 0;
}
