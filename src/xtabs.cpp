/**
 * This program requires the mapping of the categorical variables to group by to an integer range for the functions to be executed. After the
 * functions are executed the same mapping can be reversed, of course. All functions in this program use at least one categorical variable to 
 * group by.
 */

#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;
const int ALICE_CAT_LEN = 4;
const int BOB_CAT_LEN = 4;


/**
 * Simplest of the functions
 */
void test_sum(int party, string inputs[], int input_len) {
	Integer *a = new Integer[input_len];
	Integer *b = new Integer[input_len];
	Integer sums [ALICE_CAT_LEN];		// TODO: change to dynamic size
	Integer categories [ALICE_CAT_LEN];

	// Initialize the secure integers
	for (int i = 0; i < input_len; ++i) {
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
		b[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);
	}

	// Initialize sums
	for (int i = 0; i < ALICE_CAT_LEN; ++i) {
		sums[i] = Integer(BITSIZE, 0, PUBLIC);
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	// Calculate sums
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < input_len; ++i) {
		for (int j = 0; j < ALICE_CAT_LEN; ++j) {
			// This compares the given category against the category of the current element
			// The category of the element must be mapped to an integer to have less of a headache
			// if a[i] == j then result = b[i] else result = 0
			Bit eqcat = a[i].equal(categories[j]);
			Integer result = zero.select(eqcat, b[i]);	
		
			sums[j] = sums[j] + result;
		}	
	}

	// Reveal sums
    for (int i = 0; i < ALICE_CAT_LEN; ++i) {
        cout << "sum " << i << ": " << sums[i].reveal<int>() << endl;
   }

   delete[] a;
   delete[] b;
}


/**
 * For the average function, we need to use emp::Float types instead of emp::Integer types for the final results if we want precision in the 
 * averages (otherwise we can use just integer division ig).
 */
void test_average(int party, string inputs[], int input_len) {
	// CAN POTENTIALLY BE OPTIMIZED BY REVEALING THE SUM AND COUNT ALLOWING TO WORK WITH ONLY INTEGERS AND MAKING THE FLOAT DIVISION IN CLEARTEXT.
	Integer *a = new Integer[input_len];
	Float *b = new Float[input_len];
	Float sums [ALICE_CAT_LEN];		// TODO: change to dynamic size
	Float counts [ALICE_CAT_LEN];	// TODO: change to dynamic size
	Integer categories [ALICE_CAT_LEN];

	for (int i = 0; i < input_len; ++i) {
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
		b[i] = Float(stoi(inputs[i]), BOB);
	}	

	for (int i = 0; i < ALICE_CAT_LEN; ++i) {
		sums[i] = Float();
		counts[i] = Float();
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	// Calculate averages
	Float zero = Float();	// Default party is PUBLIC
	Float one = Float(1, PUBLIC);
	for (int i = 0; i < input_len; ++i) {
		for (int j = 0; j < ALICE_CAT_LEN; ++j) {
			// This compares the given category against the category of the current element
			// The category of the element must be mapped to an integer to have less of a headache
			// if a[i] == j then result = b[i] else result = 0
			Bit eqcat = a[i].equal(categories[j]);
			Float result_sum = zero.If(eqcat, b[i]);
			Float result_count = zero.If(eqcat, one);
		
			sums[j] = sums[j] + result_sum;
			counts[j] = counts[j] + result_count;
		}	
	}

	// Reveal averages
    for (int i = 0; i < ALICE_CAT_LEN; ++i) {
		float average = (sums[i] / counts[i]).reveal<double>();
        cout << "average " << i << ": " << average << endl;
	}

	delete[] a;
	delete[] b;
}


/**
 * For the mode function, both the categorical variable used to groupby and the catogorical variable of the values must be mapped
 * to integers ranging from 0 to the needed range. At the moment does not work with continuous variables for both (why would it tbf).~
 * 
 * Note: At the moment, if multiple value categories are fit to be the mode, then the last one numerically (according to the mapping) will be the 
 * one displayed.
 */
void test_mode(int party, string inputs[], int input_len) {
	Integer *a = new Integer[input_len];
	Integer *b = new Integer[input_len];
	Integer frequencies[ALICE_CAT_LEN][BOB_CAT_LEN]; //TODO: change to dynamic size
	Integer modes[ALICE_CAT_LEN];
	Integer categories_a [ALICE_CAT_LEN];
	Integer categories_b [BOB_CAT_LEN];

	// Initialize the secure integers
	for(int i = 0; i < input_len; ++i) {
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
		b[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);
	}

	// Initialize frequency count
	for(int i = 0; i < ALICE_CAT_LEN; ++i) {
		for (int j = 0; j < BOB_CAT_LEN; ++j)
			frequencies[i][j] = Integer(BITSIZE, 0);
	}

	// Initialize categories
	for(int i = 0; i < ALICE_CAT_LEN; ++i) 
		categories_a[i] = Integer(BITSIZE, i, PUBLIC);

	
	for (int i = 0; i < BOB_CAT_LEN; ++i)
		categories_b[i] = Integer(BITSIZE, i, PUBLIC);


	// Calculate frequencies of each item by group
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	Integer one (BITSIZE, 1);
	for (int i = 0; i < input_len; ++i) {
		for (int j = 0; j < ALICE_CAT_LEN; ++j) {
			// This compares the given category against the category of the current element
			// The category of the element must be mapped to an integer to have less of a headache
			// if a[i] == j then result = b[i] else result = 0 (because we use 0 as the start value)
			Bit eq_groupby_cat = a[i].equal(categories_a[j]);
			Integer result_groupby = zero.select(eq_groupby_cat, one);
			
			for (int k = 0; k < BOB_CAT_LEN; ++k) {
				Bit eq_val_cat = b[i].equal(categories_b[k]);
				Integer result_val = zero.select(eq_val_cat, result_groupby);

				frequencies[j][k] = frequencies[j][k] + result_val;
			}	
		}	
	}

	
	// With the frequencies calculated, find the mode for each group
	for (int i = 0; i < ALICE_CAT_LEN; ++i) {
		Integer max(BITSIZE, 0);
		Integer mode(BITSIZE, -1);
		for (int j = 0; j < BOB_CAT_LEN; ++j) {
			Integer freq = frequencies[i][j];
			Bit geq = freq.geq(max);

			// This will only update max when freq is greater or equal to max
			max = max.select(geq, freq);

			// If max was assigned a new value, we need to update the mode for the group
			Bit eq_max = freq.equal(max);
			mode = mode.select(eq_max, categories_b[j]);
		}
		modes[i] = mode;		
	}


	for (int i = 0; i < ALICE_CAT_LEN; ++i) {
		for (int j = 0; j < BOB_CAT_LEN; ++j) {
			cout << "group " << i <<  ", frequency of the value " << j << ": " << frequencies[i][j].reveal<int>() << endl;
		}
		cout << "mode of group " << i << ": " << modes[i].reveal<int>() << endl;
	}

	delete[] a;
	delete[] b;
}


int main(int argc, char **argv) {
	if (argc != 5 && argc != 6) {
		cout << "Usage for Alice (server): <program> 1 <port> <aggregation> <input file>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <aggregation> <input file>" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	char* aggregation = argv[argc - 2];
	char* filename = argv[argc - 1];
	
	NetIO * io = new NetIO(ip, port);
	auto ctx = setup_semi_honest(io, party);
	
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
	switch (aggregation[0]) {
		case 's':
			test_sum(party, inputs.data(), inputs.size());
			break;
		case 'a':
			ctx->set_batch_size(1024*1024);	// I assume this makes the process faster when working with floats
			test_average(party, inputs.data(), inputs.size());
			break;
		case 'm':
			test_mode(party, inputs.data(), inputs.size());
			break;
		case 'v':
			cout << "Variance" << endl;
			cout << "NOT IMPLEMENTED" << endl;
			break;
		case 'd':
			cout << "Standard Deviation" << endl;
			cout << "NOT IMPLEMENTED" << endl;
			break;
		case 'c':
			cout << "Covariance" << endl;
			cout << "NOT IMPLEMENTED" << endl;
			break;
		case 'p':	// related to stdev and covariance
			cout << "Pearson Correlation Coefficient" << endl;
			cout << "NOT IMPLEMENTED" << endl;
			break;
		default:
			cout << "Invalid aggregation type" << endl;
			break;
	}
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
	cout << "Execution time of xtabs (" << aggregation[0] << ") with: " << inputs.size() << " elements: " << duration << " ms" << endl;
	
	delete io;
    return 0;
}
