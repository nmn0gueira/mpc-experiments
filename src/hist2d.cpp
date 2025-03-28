#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;
const int INPUT_LEN = 10;

const int NUM_BINS_X = 5;
const int NUM_BINS_Y = 5;


void calc_extremeties(Float * arr, int size, Float & min, Float & max) {
	// We need to determine the max and min values to determine the bins for each coordinate
	for (int i = 0; i < size; ++i) {
		// If the value is not less or equal than max_x than it is greater than max_x
		Bit gt = !arr[i].less_equal(max);
		max = max.If(gt, arr[i]);

		Bit lt = arr[i].less_than(min);
		min = min.If(lt, arr[i]);
	}
}

void linspace(Float * arr, int size, Float & min, Float & max) {
	Float step = (max - min) / Float(size - 1, PUBLIC);
	arr[0] = min;
	for (int i = 1; i < size - 1; ++i) {
		arr[i] = arr[i-1] + step;
	}
	arr[size - 1] = max;

}

/**
 * Although the naming is borrowed from np.digitize, the functionality is slightly different. This function
 * takes bin edges as input and returns an index adjusted for zero-indexing.
 */
void digitize(Float val, Integer * bins, Float * bin_edges, int num_edges, Integer & bin_to_index) {
	static Bit false_bit = Bit();
	Bit found_index = Bit();
	for (int i = 1; i < num_edges; ++i) {	// Starts at 1 since it is useless to do a leq compare against the min edge
		Bit leq = val.less_equal(bin_edges[i]);

		// The select bit will only be true if leq is true AND the correct index has not been found yet
		Bit select = false_bit.select(!found_index, leq);
		bin_to_index = bin_to_index.select(select, bins[i - 1]);	// i - 1 since num_bins = num_edges - 1

		// Only updates found_index the first time
		found_index = found_index.select(!found_index, select);

	}
}
	

/**
 * Items are placed in bins according to the formula bin[i-1] < x <= bin[i]. This function computes a 2d histogram with the aggregation performed
 * being a count. In the feature this could be any sort of aggregation ig, like in xtabs.
 */
void test_hist2d(int party, string inputs[]) {
	Float *a = new Float[INPUT_LEN];
	Float *b = new Float[INPUT_LEN];

	int num_edges_x = NUM_BINS_X + 1;
	int num_edges_y = NUM_BINS_Y + 1;
	Float *bin_edges_x = new Float[num_edges_x];
	Float *bin_edges_y = new Float[num_edges_y];

	Integer *bins_x = new Integer[NUM_BINS_X];
	Integer *bins_y = new Integer[NUM_BINS_Y];
	Integer *hist2d = new Integer[NUM_BINS_Y * NUM_BINS_X];

	for (int i = 0; i < INPUT_LEN; ++i) {
		a[i] = Float(stoi(inputs[i]), ALICE);
		b[i] = Float(stoi(inputs[i]), BOB);
	}

	for (int i = 0; i < NUM_BINS_X; ++i) {
		bins_x[i] = Integer(BITSIZE, i , PUBLIC);
	}

	for (int i = 0; i < NUM_BINS_Y; ++i) {
		bins_y[i] = Integer(BITSIZE, i , PUBLIC);
	}

	for (int y = 0; y < NUM_BINS_Y; ++y) {
		for (int x = 0; x < NUM_BINS_X; ++x) {
			hist2d[y * NUM_BINS_X + x] = Integer(BITSIZE, 0);
		}
	}

	Float max_x(numeric_limits<float>::min(), PUBLIC);
	Float min_x(numeric_limits<float>::max(), PUBLIC);
	Float max_y(numeric_limits<float>::min(), PUBLIC);
	Float min_y(numeric_limits<float>::max(), PUBLIC);
	calc_extremeties(a, INPUT_LEN, min_x, max_x);
	calc_extremeties(b, INPUT_LEN, min_y, max_y);

	linspace(bin_edges_x, num_edges_x, min_x, max_x);
	linspace(bin_edges_y, num_edges_y, min_y, max_y);

	Integer zero(BITSIZE, 0);
	Integer one(BITSIZE, 1);
	for (int i = 0; i < INPUT_LEN; ++i) {
		Float x_val = a[i];
		Float y_val = b[i];
		Integer x_bin(BITSIZE, 0, PUBLIC);
		Integer y_bin(BITSIZE, 0 , PUBLIC);

		digitize(x_val, bins_x, bin_edges_x, num_edges_x, x_bin);

		digitize(y_val, bins_y, bin_edges_y, num_edges_y, y_bin);

		// Update histogram
		for (int y = 0; y < NUM_BINS_Y; ++y) {
			for (int x = 0; x < NUM_BINS_X; ++x) {
				int hist_index = y * NUM_BINS_X + x;

				Bit eq_x = x_bin.equal(bins_x[x]);
				Bit eq_y = y_bin.equal(bins_y[y]);

				Bit eq_bin = eq_x & eq_y;

				hist2d[hist_index] = hist2d[hist_index] + zero.select(eq_bin, one);
			}
		}
	}

	
	//  Print results
	for (int i = 0; i < num_edges_x; ++i) {
		cout << "Bin Edge x (" << i << "): " << bin_edges_x[i].reveal<double>() << endl;
	}

	for (int i = 0; i < num_edges_y; ++i) {
		cout << "Bin Edge y (" << i << "): " << bin_edges_y[i].reveal<double>() << endl;
	}

	for (int y = 0; y < NUM_BINS_Y; ++y) {
		for (int x = 0; x < NUM_BINS_X; ++x) {
			cout << "Hist2d (" << x << ", " << y << "): " << hist2d[y * NUM_BINS_X + x].reveal<int>() << endl;
		}
	}

	delete[] a;
	delete[] b;
	delete[] bin_edges_x;
	delete[] bin_edges_y;
	delete[] hist2d;
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
