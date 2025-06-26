#include "../emp/emp-sh2pc/emp-sh2pc/emp-sh2pc.h"
#include "../utils.hpp"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;


void initialize_edges(Integer * bin_edges, int num_edges, ifstream & infile, string & line) {
	int previous = numeric_limits<int>::min();
	for (int i = 0; i < num_edges; ++i) {
		getline(infile, line);
		int element = stoi(line);
		if (element <= previous) {
			cerr << "Error: bin edges must be in ascending order." << endl;
			exit(1);
		} 	
		previous = element;
		bin_edges[i] = Integer(BITSIZE, element, PUBLIC);
	}
}

void initialize_edges(Float * bin_edges, int num_edges, ifstream & infile, string & line) {
	float previous = numeric_limits<float>::lowest();
	for (int i = 0; i < num_edges; ++i) {
		getline(infile, line);
		float element = stof(line);
		if (element <= previous) {
			cerr << "Error: bin edges must be in ascending order." << endl;
			exit(1);
		}
		previous = element;
		bin_edges[i] = Float(element, PUBLIC);
	}
}

template <typename T>
void initialize_edges(T * bin_edges_x, T * bin_edges_y, int num_edges_x, int num_edges_y) {
	ifstream infile_x = utils::get_input_file('1');
	ifstream infile_y = utils::get_input_file('2');
	string line;
	initialize_edges(bin_edges_x, num_edges_x, infile_x, line);
	initialize_edges(bin_edges_y, num_edges_y, infile_y, line);
}

/**
 * Although the naming is borrowed from np.digitize, the functionality is slightly different. This function
 * takes bin edges as input and "returns" an index adjusted for zero-indexing.
 */
void digitize(Integer val, Integer * bins, Integer * bin_edges, int num_edges, Integer & bin_to_index) {
	for (int i = num_edges - 1; i > 0; --i) {
		bin_to_index = bin_to_index.select(val <= bin_edges[i], bins[i - 1]);	// i - 1 since num_bins = num_edges - 1
	}
}

/**
 * Although the naming is borrowed from np.digitize, the functionality is slightly different. This function
 * takes bin edges as input and "returns" an index adjusted for zero-indexing.
 */
void digitize(Float val, Integer * bins, Float * bin_edges, int num_edges, Integer & bin_to_index) {
	for (int i = num_edges - 1; i > 0; --i) {
		bin_to_index = bin_to_index.select(val.less_equal(bin_edges[i]), bins[i - 1]);	// i - 1 since num_bins = num_edges - 1
	}
}

void reveal_hist2d(Integer* hist2d, int num_bins_x, int num_bins_y) {
	for (int y = 0; y < num_bins_y; ++y) {
		for (int x = 0; x < num_bins_x; ++x) {
			cout << "Hist2d (" << x << ", " << y << "): " << hist2d[y * num_bins_x + x].reveal<int>() << endl;
		}
	}
}
	
/**
 * Items are placed in bins according to the formula bin[i-1] < x <= bin[i]. This function computes a 2d histogram with the aggregation performed
 * being a count. In the future this could be any sort of aggregation ig, like in xtabs.
 */
template<typename T>
void test_hist2d(int party, int input_size, int num_edges_x, int num_edges_y) {
	T *a = new T[input_size];
	T *b = new T[input_size];

	int num_bins_x = num_edges_x - 1;
	int num_bins_y = num_edges_y - 1;
	T bin_edges_x[num_edges_x];
	T bin_edges_y[num_edges_y];

	Integer bins_x[num_bins_x];
	Integer bins_y[num_bins_y];
	Integer hist2d[num_bins_y * num_bins_x];

	utils::initialize_parties<T>(party, a, b, input_size);

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
	
	initialize_edges<T>(bin_edges_x, bin_edges_y, num_edges_x, num_edges_y);

	Integer zero(BITSIZE, 0);
	for (int i = 0; i < input_size; ++i) {
		Integer x_bin(BITSIZE, 0, PUBLIC);
		Integer y_bin(BITSIZE, 0 , PUBLIC);

		digitize(a[i], bins_x, bin_edges_x, num_edges_x, x_bin);
		digitize(b[i], bins_y, bin_edges_y, num_edges_y, y_bin);

		// Update histogram
		for (int y = 0; y < num_bins_y; ++y) {
			Bit eq_y = y_bin.equal(bins_y[y]);

			for (int x = 0; x < num_bins_x; ++x) {
				int hist_index = y * num_bins_x + x;
				Bit eq_bin = x_bin.equal(bins_x[x]) & eq_y;
				// hist2d[hist_index] = hist2d[hist_index] + 1;
				emp::add_full(hist2d[hist_index].bits.data(), nullptr, hist2d[hist_index].bits.data(), zero.bits.data(), &eq_bin, hist2d[hist_index].size());

			}
		}
	}

	reveal_hist2d(hist2d, num_bins_x, num_bins_y);

	delete[] a;
	delete[] b;
}


int main(int argc, char **argv) {
	if (argc != 8 && argc != 9) {
		cout << "Usage for Alice (server): <program> 1 <port> <input_size> <mode> <num_edges_x> <num_edges_y> <input_dir>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <input_size> <mode> <num_edges_x> <num_edges_y> <input_dir>" << endl;
		cout << endl;
		cout << "Additional argument explanation: " << endl;
		cout << "Modes: i - integer, f - float" << endl;
		cout << "num_edges_x: number of edges for the x-axis (e.g. 6 edges -> 5 bins). These edges are stored in <input_dir>/1.dat" << endl;
		cout << "num_edges_y: number of edges for the y-axis (e.g. 6 edges -> 5 bins). These edges are stored in <input_dir>/2.dat" << endl;
		cout << "Input directory: directory containing the private input (0.dat) and the public bin edges files (1.dat and 2.dat)" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	int input_size = atoi(argv[argc - 5]);
	char* mode = argv[argc - 4];
	int num_edges_x = atoi(argv[argc - 3]);
	int num_edges_y = atoi(argv[argc - 2]);
	if (num_edges_x < 2 || num_edges_y < 2) {
		cerr << "Error: num_edges_x and num_edges_y must be at least 2." << endl;
		return 1;
	}
	utils::set_directory(argv[argc - 1]);

	HighSpeedNetIO * io = new HighSpeedNetIO(ip, port, port + 1);
	auto ctx = setup_semi_honest(io, party);
	ctx->set_batch_size(1024*1024);

	cout << "Party: " << (party == ALICE ? "Alice" : "Bob") << endl;
	cout << "Input size: " << input_size << endl;
	cout << "Mode: " << mode << endl;
	cout << "Number of edges for x-axis: " << num_edges_x << endl;
	cout << "Number of edges for y-axis: " << num_edges_y << endl;
	cout << "Input directory: " << utils::get_directory() << endl;

	if (mode[0] == 'i') {
		cout << "Running integer mode..." << endl;
		utils::time_it(test_hist2d<Integer>, party, input_size, num_edges_x, num_edges_y);
	} else {
		cout << "Running float mode..." << endl;
		utils::time_it(test_hist2d<Float>, party, input_size, num_edges_x, num_edges_y);
	}

	finalize_semi_honest();

	utils::print_io_stats(*io, party);
	delete io;
	
    return 0;
}
