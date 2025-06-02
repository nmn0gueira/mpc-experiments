#include "emp-sh2pc/emp-sh2pc.h"
#include "../utils.hpp"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;

const int NUM_BINS_X = 5;
const int NUM_BINS_Y = 5;

void calc_extremeties(Integer * arr, int size, Integer & min, Integer & max) {
	// We need to determine the max and min values to determine the bins for each coordinate
	for (int i = 0; i < size; ++i) {
		Bit gt = arr[i] > max;
		max = max.select(gt, arr[i]);

		Bit lt = arr[i] < min;
		min = min.select(lt, arr[i]);
	}
}

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

void linspace(Integer * arr, int size, Integer min, Integer max) {
	Integer step = (max - min) / Integer(BITSIZE, size - 1);
	arr[0] = min;
	for (int i = 1; i < size - 1; ++i) {
		arr[i] = arr[i-1] + step;
	}
	arr[size - 1] = max;
}

void linspace(Float * arr, int size, Float min, Float max) {
	Float step = (max - min) / Float(size - 1, PUBLIC);
	arr[0] = min;
	for (int i = 1; i < size - 1; ++i) {
		arr[i] = arr[i-1] + step;
	}
	arr[size - 1] = max;
}

/**
 * Although the naming is borrowed from np.digitize, the functionality is slightly different. This function
 * takes bin edges as input and "returns" an index adjusted for zero-indexing.
 */
void digitize(Integer val, Integer * bins, Integer * bin_edges, int num_edges, Integer & bin_to_index) {
	static Bit false_bit = Bit();
	Bit found_index = Bit();
	for (int i = 1; i < num_edges; ++i) {	// Starts at 1 since it is useless to do a leq compare against the min edge
		Bit leq = val <= bin_edges[i];

		// The select bit will only be true if leq is true AND the correct index has not been found yet
		Bit select = false_bit.select(!found_index, leq);
		bin_to_index = bin_to_index.select(select, bins[i - 1]);	// i - 1 since num_bins = num_edges - 1

		// Only updates found_index the first time
		found_index = found_index.select(!found_index, select);

	}
}

/**
 * Although the naming is borrowed from np.digitize, the functionality is slightly different. This function
 * takes bin edges as input and "returns" an index adjusted for zero-indexing.
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

void initialize_parties(Integer * alice, Integer * bob, string inputs[], int input_len) {
	for (int i = 0; i < input_len; ++i) {
		alice[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
	}

	for (int i = 0; i < input_len; ++i) {
		bob[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);
	}
}

void initialize_parties(Float * alice, Float * bob, string inputs[], int input_len) {
	for (int i = 0; i < input_len; ++i) {
		alice[i] = Float(stoi(inputs[i]), ALICE);
	}

	for (int i = 0; i < input_len; ++i) {
		bob[i] = Float(stoi(inputs[i]), BOB);
	}
}

void initialize_extremeties(Integer &max_x, Integer &min_x, Integer &max_y, Integer &min_y) {
	max_x = Integer(BITSIZE, numeric_limits<int>::min(), PUBLIC);
	min_x = Integer(BITSIZE, numeric_limits<int>::max(), PUBLIC);
	max_y = Integer(BITSIZE, numeric_limits<int>::min(), PUBLIC);
	min_y = Integer(BITSIZE, numeric_limits<int>::max(), PUBLIC);
}

void initialize_extremeties(Float &max_x, Float &min_x, Float &max_y, Float &min_y) {
	max_x = Float(numeric_limits<float>::min(), PUBLIC);
	min_x = Float(numeric_limits<float>::max(), PUBLIC);
	max_y = Float(numeric_limits<float>::min(), PUBLIC);
	min_y = Float(numeric_limits<float>::max(), PUBLIC);
}

void reveal_bin_edges(Float * bin_edges_x, Float * bin_edges_y, int num_edges_x, int num_edges_y) {
	for (int i = 0; i < num_edges_x; ++i) {
		cout << "Bin Edge x (" << i << "): " << bin_edges_x[i].reveal<double>() << endl;
	}

	for (int i = 0; i < num_edges_y; ++i) {
		cout << "Bin Edge y (" << i << "): " << bin_edges_y[i].reveal<double>() << endl;
	}
}

void reveal_bin_edges(Integer * bin_edges_x, Integer * bin_edges_y, int num_edges_x, int num_edges_y) {
	for (int i = 0; i < num_edges_x; ++i) {
		cout << "Bin Edge x (" << i << "): " << bin_edges_x[i].reveal<int>() << endl;
	}

	for (int i = 0; i < num_edges_y; ++i) {
		cout << "Bin Edge y (" << i << "): " << bin_edges_y[i].reveal<int>() << endl;
	}
}

void reveal_hist2d(Integer* hist2d, int num_bins_x, int num_bins_y) {
	for (int y = 0; y < num_bins_y; ++y) {
		for (int x = 0; x < num_bins_x; ++x) {
			int hist_value =  hist2d[y * num_bins_x + x].reveal<int>();
			//total_sum += hist_value;
			cout << "Hist2d (" << x << ", " << y << "): " << hist_value << endl;
		}
	}
}
	
/**
 * Items are placed in bins according to the formula bin[i-1] < x <= bin[i]. This function computes a 2d histogram with the aggregation performed
 * being a count. In the feature this could be any sort of aggregation ig, like in xtabs.
 */
template<typename T>
void test_hist2d(string inputs[], int input_len, int num_bins_x=NUM_BINS_X, int num_bins_y=NUM_BINS_Y) {
	T *a = new T[input_len];
	T *b = new T[input_len];

	int num_edges_x = num_bins_x + 1;
	int num_edges_y = num_bins_y + 1;
	T bin_edges_x[num_edges_x];
	T bin_edges_y[num_edges_y];

	Integer bins_x[num_bins_x];
	Integer bins_y[num_bins_y];
	Integer hist2d[num_bins_y * num_bins_x];

	initialize_parties(a, b, inputs, input_len);

	for (int i = 0; i < num_bins_x; ++i) {
		bins_x[i] = Integer(BITSIZE, i , PUBLIC);
	}

	for (int i = 0; i < num_bins_y; ++i) {
		bins_y[i] = Integer(BITSIZE, i , PUBLIC);
	}

	for (int y = 0; y < num_bins_y; ++y) {
		for (int x = 0; x < num_bins_x; ++x) {
			hist2d[y * num_bins_x + x] = Integer(BITSIZE, 0);
		}
	}
	T max_x;
	T min_x;
	T max_y;
	T min_y;
	initialize_extremeties(max_x, min_x, max_y, min_y);
	
	calc_extremeties(a, input_len, min_x, max_x);
	calc_extremeties(b, input_len, min_y, max_y);

	linspace(bin_edges_x, num_edges_x, min_x, max_x);
	linspace(bin_edges_y, num_edges_y, min_y, max_y);

	Integer zero(BITSIZE, 0);
	for (int i = 0; i < input_len; ++i) {
		T x_val = a[i];
		T y_val = b[i];
		Integer x_bin(BITSIZE, 0, PUBLIC);
		Integer y_bin(BITSIZE, 0 , PUBLIC);

		digitize(x_val, bins_x, bin_edges_x, num_edges_x, x_bin);
		digitize(y_val, bins_y, bin_edges_y, num_edges_y, y_bin);

		// Update histogram
		for (int y = 0; y < num_bins_y; ++y) {
			for (int x = 0; x < num_bins_x; ++x) {
				int hist_index = y * num_bins_x + x;

				Bit eq_x = x_bin.equal(bins_x[x]);
				Bit eq_y = y_bin.equal(bins_y[y]);

				Bit eq_bin = eq_x & eq_y;
				// hist2d[hist_index] = hist2d[hist_index] + 1;
				emp::add_full(hist2d[hist_index].bits.data(), nullptr, hist2d[hist_index].bits.data(), zero.bits.data(), &eq_bin, hist2d[hist_index].size());

			}
		}
	}

	reveal_bin_edges(bin_edges_x, bin_edges_y, num_edges_x, num_edges_y);
	reveal_hist2d(hist2d, num_bins_x, num_bins_y);

	delete[] a;
	delete[] b;
}


int main(int argc, char **argv) {
	if (argc != 5 && argc != 6) {
		cout << "Usage for Alice (server): <program> 1 <port> <mode> <input file>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <mode> <input file>" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	char* mode = argv[argc - 2];
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

	if (mode[0] == 'i') {
		cout << "Running integer mode..." << endl;
		utils::time_it(test_hist2d<Integer>, inputs.data(), inputs.size(), NUM_BINS_X, NUM_BINS_Y);
	} else {
		cout << "Running float mode..." << endl;
		utils::time_it(test_hist2d<Float>, inputs.data(), inputs.size(), NUM_BINS_X, NUM_BINS_Y);
	}

	finalize_semi_honest();

	utils::print_io_stats(*io, party);
	delete io;
	
    return 0;
}
