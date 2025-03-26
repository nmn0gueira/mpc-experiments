#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;
const int INPUT_LEN = 10;

const int NUM_BINS_X = 5;
const int NUM_BINS_Y = 5;


void calc_extremeties(Float * arr, int size, Float & max, Float & min) {
	// We need to determine the max and min values to determine the bins for each coordinate
	for (int i = 0; i < INPUT_LEN; ++i) {
		// If the value is not less or equal than max_x than it is greater than max_x
		Bit gt = !arr[i].less_equal(max);
		max = max.If(gt, arr[i]);

		Bit lt = arr[i].less_than(min);
		min = min.If(lt, arr[i]);
	}
}
	

/**
 * Single variable linear regression. Assumes Alice has the feature column and Bob has the labels.
 */
void test_hist2d(int party, string inputs[], bool scale=true) {
	Float *a = new Float[INPUT_LEN];
	Float *b = new Float[INPUT_LEN];
	Float input_size = Float(INPUT_LEN, PUBLIC);
	Float *bins_x = new Float[NUM_BINS_X];
	Float *bins_y = new Float[NUM_BINS_Y];
	Float *hist2d = new Float[NUM_BINS_X * NUM_BINS_Y];

	for (int i = 0; i < INPUT_LEN; ++i) {
		a[i] = Float(stoi(inputs[i]), ALICE);
		b[i] = Float(stoi(inputs[i]), BOB);
	}

	Float max_x(numeric_limits<float>::min(), PUBLIC);
	Float min_x(numeric_limits<float>::max(), PUBLIC);
	Float max_y(numeric_limits<float>::min(), PUBLIC);
	Float min_y(numeric_limits<float>::max(), PUBLIC);
	calc_extremeties(a, INPUT_LEN, max_x, min_x);
	calc_extremeties(b, INPUT_LEN, max_y, min_y);

	cout << "Max x: " << max_x.reveal<double>() << " Min x: " << min_x.reveal<double>() << endl;
	cout << "Max y: " << max_y.reveal<double>() << " Min y: " << min_y.reveal<double>() << endl;
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
	
	string inputs[INPUT_LEN];
	for(int i = 0; i < INPUT_LEN; i++) {
		getline(infile, inputs[i]);
	}
	infile.close();

	test_hist2d(party, inputs);
	
	delete io;
    return 0;
}
