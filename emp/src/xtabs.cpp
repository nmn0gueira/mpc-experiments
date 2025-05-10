/**
 * This program requires the mapping of the categorical variables to group by to an integer range for the functions to be executed. After the
 * functions are executed the same mapping can be reversed, of course. All functions in this program use at least one categorical variable to 
 * group by.
 * 
 * Note: It is possible to extend the functions to work with a variable amount of categorical variables if using recursion for the comparison loops (if emp-toolkit allows it).
 */

#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <dirent.h>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;
const int CAT_LEN = 4;	// For now, the number of categories is fixed to 4 (0, 1, 2, 3)


void initialize_groupby_inputs(int party, Integer *agg_by, vector<vector<string>> inputs, char* agg_cols) {
	// Count the number of aggregation columns and the number of columns for Alice and Bob
	int sample_size = inputs[0].size();
	int agg_cols_len = strlen(agg_cols);	// Number of characters in the string (NOT THE ACTUAL NUMBER OF COLUMNS)
	const int STEP = 2;	// Each column is represented by two characters (e.g. a0, b1, etc.)

	for (int i = 0; i < agg_cols_len; i += STEP) {
		int input_sequence_num = agg_cols[i + 1] - '0';	// Convert char to int

		if (agg_cols[i] == 'a') {
			for (int j = 0; j < sample_size; ++j) {
				int agg_by_index = i / STEP * sample_size + j;
				int input = party == ALICE ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Alice will have the input value
				agg_by[agg_by_index] = Integer(BITSIZE, input, ALICE);
			}
		}

		else if (agg_cols[i] == 'b') {
			for (int j = 0; j < sample_size; ++j) {
				int agg_by_index = i / STEP * sample_size + j;
				int input = party == BOB ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Bob will have the input value
				agg_by[agg_by_index] = Integer(BITSIZE, input, BOB);
			}	
		}

		else {
			cout << "Invalid aggregation column" << endl;
			exit(1);
		}
	}
}

void initialize_values(int party, Integer *values, vector<vector<string>> inputs, char* value_col) {
	int sample_size = inputs[0].size();
	int input_sequence_num = value_col[1] - '0';	// Convert char to int
	// Count the number of aggregation columns and the number of columns for Alice and Bob
	if (value_col[0] == 'a') {
		for (int j = 0; j < sample_size; ++j) {
			int input = party == ALICE ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Alice will have the input value
			values[j] = Integer(BITSIZE, input, ALICE);
		}
	}
	else if (value_col[0] == 'b') {
		for (int j = 0; j < sample_size; ++j) {
			int input = party == BOB ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Bob will have the input value
			values[j] = Integer(BITSIZE, input, BOB);
		}	
	}
	else {
		cout << "Invalid value column" << endl;
		exit(1);
	}
}

void test_sum1(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col) {
	int sample_size = inputs[0].size();
	Integer *agg_by = new Integer[inputs[0].size()];		//  May contain inputs of both parties
	Integer *values = new Integer[inputs[0].size()];
	
	Integer sums[CAT_LEN];	// If CAT_LEN was not fixed, each dimension would be initialized to the respective number of categories
	Integer categories[CAT_LEN];
	initialize_groupby_inputs(party, agg_by, inputs, agg_cols);
	initialize_values(party, values, inputs, value_col);

	// Initialize sums
	for (int i = 0; i < CAT_LEN; ++i) {
		sums[i] = Integer(BITSIZE, 0, PUBLIC);
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	// Calculate sums
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
			// This compares the given category against the category of the current element
			// The category of the element must be mapped to an integer to have less of a headache
			// if a[i] == j then result = b[i] else result = 0
			Bit eqcat = agg_by[i].equal(categories[j]);
			Integer result = zero.select(eqcat, values[i]);	
		
			sums[j] = sums[j] + result;
		}	
	}

	// Reveal sums
    for (int i = 0; i < CAT_LEN; ++i) {
        cout << "sum " << i << ": " << sums[i].reveal<int>() << endl;
   }

   delete[] agg_by;
   delete[] values;
   //delete[] categories;
   //delete[] sums;
}


/**
 * Same as the test_sum function, but now we are grouping by two categorical variables.
 */
void test_sum2(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col) {
	int sample_size = inputs[0].size();
	Integer *agg_by = new Integer[inputs[0].size() * 2];		//  May contain inputs of both parties
	Integer *values = new Integer[inputs[0].size()];
	
	Integer sums[CAT_LEN][CAT_LEN];	// If CAT_LEN was not fixed, each dimension would be initialized to the respective number of categories
	Integer categories[CAT_LEN][2];

	initialize_groupby_inputs(party, agg_by, inputs, agg_cols);
	initialize_values(party, values, inputs, value_col);

	for (int i = 0; i < CAT_LEN; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
			sums[i][j] = Integer(BITSIZE, 0, PUBLIC);
		}
	}

	for (int i = 0; i < CAT_LEN; ++i) {
		categories[i][0] = Integer(BITSIZE, i, PUBLIC);
		categories[i][1] = Integer(BITSIZE, i, PUBLIC);
	}

	// Calculate sums
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
			Bit eq_first_cat = agg_by[i].equal(categories[j][0]);
			Integer result_first_cat = zero.select(eq_first_cat, values[i]);
			
			for (int k = 0; k < CAT_LEN; ++k) {
				Bit eq_second_cat = agg_by[sample_size + i].equal(categories[k][1]);
				Integer result_val = zero.select(eq_second_cat, result_first_cat);

				sums[j][k] = sums[j][k] + result_val;	// Only if both categories match do we add the value (otherwise adds 0)
			}	
		}	
	}
	
	// Reveal sums
	for (int i = 0; i < CAT_LEN; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
			cout << "Sum (" << i << ", " << j << "): " << sums[i][j].reveal<int>() << endl;
		}
	}

   delete[] agg_by;
   delete[] values;
}


/**
 * For the average function, we need to use emp::Float types instead of emp::Integer types for the final results if we want precision in the 
 * averages (otherwise we can use just integer division ig).
 */
void test_average(int party, string inputs[], int input_len) {
	Integer *a = new Integer[input_len];
	Float *b = new Float[input_len];
	Float sums [CAT_LEN];		// TODO: change to dynamic size
	Float counts [CAT_LEN];	// TODO: change to dynamic size
	Integer categories [CAT_LEN];

	for (int i = 0; i < input_len; ++i) {
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
		b[i] = Float(stoi(inputs[i]), BOB);
	}	

	for (int i = 0; i < CAT_LEN; ++i) {
		sums[i] = Float();
		counts[i] = Float();
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	// Calculate averages
	Float zero = Float();	// Default party is PUBLIC
	Float one = Float(1, PUBLIC);
	for (int i = 0; i < input_len; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
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
    for (int i = 0; i < CAT_LEN; ++i) {
		float average = (sums[i] / counts[i]).reveal<double>();
        cout << "average " << i << ": " << average << endl;
	}

	delete[] a;
	delete[] b;
}


/**
 * For the mode function, both the categorical variable used to groupby and the catogorical variable of the values must be mapped
 * to integers ranging from 0 to the needed range. At the moment does not work with continuous variables for both (why would it tbf).
 * 
 * Note: At the moment, if multiple value categories are fit to be the mode, then the last one numerically (according to the mapping) will be the 
 * one displayed.
 */
void test_mode(int party, string inputs[], int input_len) {
	Integer *a = new Integer[input_len];
	Integer *b = new Integer[input_len];
	Integer frequencies[CAT_LEN][CAT_LEN];
	Integer modes[CAT_LEN];
	Integer categories_a [CAT_LEN];
	Integer categories_b [CAT_LEN];

	// Initialize the secure integers
	for(int i = 0; i < input_len; ++i) {
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
		b[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);
	}

	// Initialize frequency count
	for(int i = 0; i < CAT_LEN; ++i) {
		for (int j = 0; j < CAT_LEN; ++j)
			frequencies[i][j] = Integer(BITSIZE, 0);
	}

	// Initialize categories
	for(int i = 0; i < CAT_LEN; ++i) 
		categories_a[i] = Integer(BITSIZE, i, PUBLIC);

	
	for (int i = 0; i < CAT_LEN; ++i)
		categories_b[i] = Integer(BITSIZE, i, PUBLIC);


	// Calculate frequencies of each item by group
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	Integer one (BITSIZE, 1);
	for (int i = 0; i < input_len; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
			// This compares the given category against the category of the current element
			// The category of the element must be mapped to an integer to have less of a headache
			// if a[i] == j then result = b[i] else result = 0 (because we use 0 as the start value)
			Bit eq_groupby_cat = a[i].equal(categories_a[j]);
			Integer result_groupby = zero.select(eq_groupby_cat, one);
			
			for (int k = 0; k < CAT_LEN; ++k) {
				Bit eq_val_cat = b[i].equal(categories_b[k]);
				Integer result_val = zero.select(eq_val_cat, result_groupby);

				frequencies[j][k] = frequencies[j][k] + result_val;
			}	
		}	
	}

	
	// With the frequencies calculated, find the mode for each group
	for (int i = 0; i < CAT_LEN; ++i) {
		Integer max(BITSIZE, 0);
		Integer mode(BITSIZE, -1);
		for (int j = 0; j < CAT_LEN; ++j) {
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


	for (int i = 0; i < CAT_LEN; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
			cout << "group " << i <<  ", frequency of the value " << j << ": " << frequencies[i][j].reveal<int>() << endl;
		}
		cout << "mode of group " << i << ": " << modes[i].reveal<int>() << endl;
	}

	delete[] a;
	delete[] b;
}


/**
 * For the mode function, both the categorical variable used to groupby and the catogorical variable of the values must be mapped
 * to integers ranging from 0 to the needed range. At the moment does not work with continuous variables for both (why would it tbf).
 * 
 * Note: At the moment, if multiple value categories are fit to be the mode, then the last one numerically (according to the mapping) will be the 
 * one displayed.
 */
void test_freq(int party, string inputs[], int input_len) {
	Integer *a = new Integer[input_len];
	Integer *b = new Integer[input_len];
	Integer frequencies [CAT_LEN][CAT_LEN];
	Integer categories_a [CAT_LEN];
	Integer categories_b [CAT_LEN];

	// Initialize the secure integers
	for(int i = 0; i < input_len; ++i) {
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);
		b[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);
	}

	// Initialize frequency count
	for(int i = 0; i < CAT_LEN; ++i) {
		for (int j = 0; j < CAT_LEN; ++j)
			frequencies[i][j] = Integer(BITSIZE, 0);
	}

	// Initialize categories
	for(int i = 0; i < CAT_LEN; ++i) 
		categories_a[i] = Integer(BITSIZE, i, PUBLIC);

	
	for (int i = 0; i < CAT_LEN; ++i)
		categories_b[i] = Integer(BITSIZE, i, PUBLIC);


	// Calculate frequencies of each item by group
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	Integer one (BITSIZE, 1);
	for (int i = 0; i < input_len; ++i) {
		for (int j = 0; j < CAT_LEN; ++j) {
			// This compares the given category against the category of the current element
			// The category of the element must be mapped to an integer to have less of a headache
			// if a[i] == j then result = b[i] else result = 0 (because we use 0 as the start value)
			Bit eq_groupby_cat = a[i].equal(categories_a[j]);
			Integer result_groupby = zero.select(eq_groupby_cat, one);
			
			for (int k = 0; k < CAT_LEN; ++k) {
				Bit eq_val_cat = b[i].equal(categories_b[k]);
				Integer result_val = zero.select(eq_val_cat, result_groupby);

				frequencies[j][k] = frequencies[j][k] + result_val;
			}	
		}	
	}

	for (int i = 0; i < CAT_LEN; ++i) {
		cout << "Group " << i << endl;
		for (int j = 0; j < CAT_LEN; ++j) {
			cout <<  "Frequency of the value " << j << ": " << frequencies[i][j].reveal<int>() << endl;
		}
	}

	delete[] a;
	delete[] b;
}


void xtabs_1(char aggregation, int party, vector<string> inputs) {

}

void xtabs_2() {

}


void test_xtabs(int party, vector<vector<string>> inputs, char aggregation, char* agg_cols, char* value_col) {
	auto start = chrono::high_resolution_clock::now();
	int num_agg_cols = strlen(agg_cols) / 2;	// Number of aggregation columns (e.g. a0b1 -> 2)
	switch (aggregation) {
		case 's':
			if (num_agg_cols == 1) {
				cout << "Sum" << endl;
				test_sum1(party, inputs, agg_cols, value_col);
			}
			else if (num_agg_cols == 2) {
				cout << "Sum" << endl;
				test_sum2(party, inputs, agg_cols, value_col);
			}
			else {
				cout << "Invalid number of aggregation columns" << endl;
			}
			break;
		case 'a':
			//ctx->set_batch_size(1024*1024);	// I assume this makes the process faster when working with floats (taken from example code)
			//test_average(party, inputs.data(), inputs.size());
			break;
		case 'm':
			//test_mode(party, inputs.data(), inputs.size());
			break;
		case 'f':
			//test_freq(party, inputs.data(), inputs.size());
			break;
		case 'd':
			cout << "Standard Deviation" << endl;
			cout << "NOT IMPLEMENTED" << endl;
			break;
		case 'v':
			cout << "Variance" << endl;
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
	cout << "Execution time of xtabs (" << aggregation << ") with " << inputs[0].size() << " elements: " << duration << " ms" << endl;

}


int main(int argc, char **argv) {
	if (argc != 7 && argc != 8) {
		cout << "Usage for Alice (server): <program> 1 <port> <aggregation> <aggregate_by> <value_col> <input dir>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <aggregation> <aggregate_by> <value_col> <input dir>" << endl;
		cout << endl;
		cout << "Additional argument explanation: " << endl;
		cout << "<aggregate_by> argument has format of a0b1 for using Alice's column 0 and Bob's column 1 to aggregate by" << endl;
		cout << "<value_col> argument has format of a0 for using Alice's column 0 as the value column" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	
	char* aggregation = argv[argc - 4];
	char* agg_cols = argv[argc - 3];
	char* value_col = argv[argc - 2];
	char* input_dir = argv[argc - 1];
	
	NetIO * io = new NetIO(ip, port);
	auto ctx = setup_semi_honest(io, party);
	ctx->set_batch_size(1024*1024);

	vector<vector<string>> input_matrix;

	// TODO: SEE IF IT IS BETTER TO ONLY READ FILES ENDING IN THE PARTY ID (A OR B) OR IF IT IS BETTER TO READ ALL FILES WHILE SPECIFYING THE PARTY IN THE ARGUMENTS
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(input_dir)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			// Skip the current directory "." and the parent directory ".."
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            string file_path = string(input_dir) + "/" + ent->d_name;

			ifstream infile(file_path);
			if (!infile.is_open()) {
				cerr << "Failed to open file: " << input_dir << endl;
				return 1;
			}
			
			vector<string> input_vector;
			string line;
			while(getline(infile, line)) {
				input_vector.push_back(line);
			}
			infile.close();

			input_matrix.push_back(input_vector);
		}
		closedir (dir);
	} else {
		// Could not open directory
		perror("Failed to open directory");
		return EXIT_FAILURE;
	}

	test_xtabs(party, input_matrix, aggregation[0], agg_cols, value_col);
	/*
	auto start = chrono::high_resolution_clock::now();
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
	cout << "Execution time of xtabs (" << aggregation[0] << ") with " << inputs.size() << " elements: " << duration << " ms" << endl;
	*/
	delete io;
    return 0;
}
