#include "emp-sh2pc/emp-sh2pc.h"
#include "utils/timing_utils.hpp"
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

void linspace(Integer * arr, int size, Integer min, Integer max) {
	Integer step = (max - min) / Integer(BITSIZE, size - 1);
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
 * Items are placed in bins according to the formula bin[i-1] < x <= bin[i]. This function computes a 2d histogram with the aggregation performed
 * being a count. In the feature this could be any sort of aggregation ig, like in xtabs.
 */
void test_hist2d(string inputs[], int input_len, int num_bins_x=NUM_BINS_X, int num_bins_y=NUM_BINS_Y) {
	Integer *a = new Integer[input_len];
	Integer *b = new Integer[input_len];

	int num_edges_x = num_bins_x + 1;
	int num_edges_y = num_bins_y + 1;
	Integer bin_edges_x[num_edges_x];
	Integer bin_edges_y[num_edges_y];

	Integer bins_x[num_bins_x];
	Integer bins_y[num_bins_y];
	Integer hist2d[num_bins_y * num_bins_x];

	for (int i = 0; i < input_len; ++i) {
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
	}

	for (int i = 0; i < input_len; ++i) {
		b[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);
	}

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

	Integer max_x(BITSIZE, numeric_limits<int>::min(), PUBLIC);
	Integer min_x(BITSIZE, numeric_limits<int>::max(), PUBLIC);
	Integer max_y(BITSIZE, numeric_limits<int>::min(), PUBLIC);
	Integer min_y(BITSIZE, numeric_limits<int>::max(), PUBLIC);
	calc_extremeties(a, input_len, min_x, max_x);
	calc_extremeties(b, input_len, min_y, max_y);

	linspace(bin_edges_x, num_edges_x, min_x, max_x);
	linspace(bin_edges_y, num_edges_y, min_y, max_y);

	Integer zero(BITSIZE, 0);
	for (int i = 0; i < input_len; ++i) {
		Integer x_val = a[i];
		Integer y_val = b[i];
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

	for (int i = 0; i < num_edges_x; ++i) {
		cout << "Bin Edge x (" << i << "): " << bin_edges_x[i].reveal<int>() << endl;
	}

	for (int i = 0; i < num_edges_y; ++i) {
		cout << "Bin Edge y (" << i << "): " << bin_edges_y[i].reveal<int>() << endl;
	}

	//int total_sum = 0;
	for (int y = 0; y < num_bins_y; ++y) {
		for (int x = 0; x < num_bins_x; ++x) {
			int hist_value =  hist2d[y * num_bins_x + x].reveal<int>();
			//total_sum += hist_value;
			cout << "Hist2d (" << x << ", " << y << "): " << hist_value << endl;
		}
	}
	//cout << "Total elements check: " << total_sum << endl;

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

	utils::time_it(test_hist2d, inputs.data(), inputs.size(), NUM_BINS_X, NUM_BINS_Y);

	const char* party_str = party == ALICE ? "Alice" : "Bob";
	cout << "Data sent (" << party_str << "): " << io->counter / (1024.0 * 1024) << " MB" << endl;

	finalize_semi_honest();
	delete io;
	
    return 0;
}
