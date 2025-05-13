/**
 * This program requires the mapping of the categorical variables to group by to an integer range for the functions to be executed. After the
 * functions are executed the same mapping can be reversed, of course. All functions in this program use at least one categorical variable to 
 * group by.
 * 
 * Note 1: Although it may be possible to extend the functions to work with a variable amount of categorical variables, the code would be a bit more
 * complex and the performance would be worse. For now, the number of categories is fixed to 4 (0, 1, 2, 3).
 * Note 2: All used columns (which are themselves from different files read) are assumed to have the same number of elements.
 */

#include "emp-sh2pc/emp-sh2pc.h"
#include "utils/timing_utils.hpp"

#include <iostream>
#include <dirent.h>
#include <unistd.h>
using namespace emp;
using namespace std;

const int BITSIZE = 32;
const int CAT_LEN = 4;	// For now, the number of categories is fixed to 4 (0, 1, 2, 3)


void initialize_groupby_inputs(int party, Integer *group_by, vector<vector<string>> inputs, char* agg_cols) {
	int sample_size = inputs[0].size();
	int agg_cols_len = strlen(agg_cols);	// Number of characters in the string (NOT THE ACTUAL NUMBER OF COLUMNS)
	const int STEP = 2;	// Each column is represented by two characters (e.g. a0, b1, etc.)

	for (int i = 0; i < agg_cols_len; i += STEP) {
		int input_sequence_num = agg_cols[i + 1] - '0';	// Convert char to int

		if (agg_cols[i] == 'a') {
			for (int j = 0; j < sample_size; ++j) {
				int group_by_index = i / STEP * sample_size + j;
				int input = party == ALICE ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Alice will have the input value
				group_by[group_by_index] = Integer(BITSIZE, input, ALICE);
			}
		}

		else if (agg_cols[i] == 'b') {
			for (int j = 0; j < sample_size; ++j) {
				int group_by_index = i / STEP * sample_size + j;
				int input = party == BOB ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Bob will have the input value
				group_by[group_by_index] = Integer(BITSIZE, input, BOB);
			}	
		}

		else {
			cout << "Invalid syntax" << endl;
			exit(1);
		}
	}
}

void initialize_values_i(int party, Integer *values, vector<vector<string>> inputs, char* value_col) {
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

void initialize_values_f(int party, Float *values, vector<vector<string>> inputs, char* value_col) {
	int sample_size = inputs[0].size();
	int input_sequence_num = value_col[1] - '0';	// Convert char to int
	// Count the number of aggregation columns and the number of columns for Alice and Bob
	if (value_col[0] == 'a') {
		for (int j = 0; j < sample_size; ++j) {
			int input = party == ALICE ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Alice will have the input value
			values[j] = Float(input, ALICE);
		}
	}
	else if (value_col[0] == 'b') {
		for (int j = 0; j < sample_size; ++j) {
			int input = party == BOB ? stoi(inputs[input_sequence_num][j]) : 0;	// Only Bob will have the input value
			values[j] = Float(input, BOB);
		}	
	}
	else {
		cout << "Invalid value column" << endl;
		exit(1);
	}
}

void test_sum1(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col, int cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size];		//  May contain inputs of both parties
	Integer *values = new Integer[sample_size];
	
	Integer sums[cat_len];	// If CAT_LEN was not fixed, each dimension would be initialized to the respective number of categories
	Integer categories[cat_len];
	initialize_groupby_inputs(party, group_by, inputs, agg_cols);
	initialize_values_i(party, values, inputs, value_col);

	for (int i = 0; i < cat_len; ++i) {
		sums[i] = Integer(BITSIZE, 0, PUBLIC);
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < cat_len; ++j) {
			// This compares the given category against the category of the current element
			// The category of the element must be mapped to an integer to have less of a headache
			// if a[i] == j then result = b[i] else result = 0
			Bit eqcat = group_by[i].equal(categories[j]);
			Integer result = zero.select(eqcat, values[i]);	
		
			sums[j] = sums[j] + result;
		}	
	}

    for (int i = 0; i < cat_len; ++i) {
        cout << "sum " << i << ": " << sums[i].reveal<int>() << endl;
   }

   delete[] group_by;
   delete[] values;
}

/**
 * Same as the test_sum1 function, but now we are grouping by two categorical variables.
 */
void test_sum2(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size * 2];	//  May contain inputs of both parties
	Integer *values = new Integer[sample_size];

	Integer sums[first_cat_len][second_cat_len];
	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, inputs, agg_cols);
	initialize_values_i(party, values, inputs, value_col);

	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			sums[i][j] = Integer(BITSIZE, 0, PUBLIC);
		}
	}

	for (int i = 0; i < first_cat_len; ++i) {
		categories_1[i] = Integer(BITSIZE, i, PUBLIC);
	}

	for (int i = 0; i < second_cat_len; ++i) {
		categories_2[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Integer zero(BITSIZE, 0);
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[sample_size + i].equal(categories_2[k]);
				Bit match = eq_first_cat & eq_second_cat;
				Integer result_val = zero.select(match, values[i]);

				sums[j][k] = sums[j][k] + result_val;	// Only if both categories match do we add the value (otherwise adds 0)
			}	
		}	
	}

	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			cout << "Sum (" << i << ", " << j << "): " << sums[i][j].reveal<int>() << endl;
		}
	}

	delete[] group_by;
	delete[] values;
}


/**
 * For the average function, we need to use emp::Float types instead of emp::Integer types for the final results if we want precision in the 
 * averages (otherwise we can use just integer division ig).
 */
void test_average1(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col, int cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size];		//  May contain inputs of both parties
	Float *values = new Float[sample_size];
	
	Float sums[cat_len];
	Float counts [cat_len];
	Integer categories[cat_len];

	initialize_groupby_inputs(party, group_by, inputs, agg_cols);
	initialize_values_f(party, values, inputs, value_col);

	for (int i = 0; i < cat_len; ++i) {
		sums[i] = Float();
		counts[i] = Float();
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Float zero = Float();
	Float one = Float(1, PUBLIC);
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < cat_len; ++j) {
			Bit eqcat = group_by[i].equal(categories[j]);
			Float result_sum = zero.If(eqcat, values[i]);
			Float result_count = zero.If(eqcat, one);	// I do not know how to optimize the sum of a bit to a float like in the fast variant
		
			sums[j] = sums[j] + result_sum;
			counts[j] = counts[j] + result_count;
		}	
	}

    for (int i = 0; i < cat_len; ++i) {
		float average = (sums[i] / counts[i]).reveal<double>();
        cout << "Average (" << i << "): " << average << endl;
	}

	delete[] group_by;
	delete[] values;
}



void test_average2(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size * 2];		//  May contain inputs of both parties
	Float *values = new Float[sample_size];
	
	Float sums[first_cat_len][second_cat_len];
	Float counts[first_cat_len][second_cat_len];
	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, inputs, agg_cols);
	initialize_values_f(party, values, inputs, value_col);

	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			sums[i][j] = Float();
			counts[i][j] = Float();
		}
	}

	for (int i = 0; i < first_cat_len; ++i) {
		categories_1[i] = Integer(BITSIZE, i, PUBLIC);
	}

	for (int i = 0; i < second_cat_len; ++i) {
		categories_2[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Float zero = Float();
	Float one = Float(1, PUBLIC);
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[sample_size + i].equal(categories_2[k]);
				Bit match = eq_first_cat & eq_second_cat;
				
				Float result_sum = zero.If(match, values[i]);
				Float result_count = zero.If(match, one);	// I do not know how to optimize the sum of a bit to a float like in the fast variant

				sums[j][k] = sums[j][k] + result_sum;
				counts[j][k] = counts[j][k] + result_count;	
			}	
		}	
	}
	
	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			cout << "Average (" << i << ", " << j << "): " << (sums[i][j] / counts[i][j]).reveal<double>() << endl;
		}
	}

   delete[] group_by;
   delete[] values;
}


void test_average1_fast(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col, int cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size];		//  May contain inputs of both parties
	Integer *values = new Integer[sample_size];
	
	Integer sums[cat_len];
	Integer counts [cat_len];
	Integer categories[cat_len];

	initialize_groupby_inputs(party, group_by, inputs, agg_cols);
	initialize_values_i(party, values, inputs, value_col);

	for (int i = 0; i < cat_len; ++i) {
		sums[i] = Integer(BITSIZE, 0, PUBLIC);
		counts[i] = Integer(BITSIZE, 0, PUBLIC);
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Integer zero(BITSIZE, 0);
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < cat_len; ++j) {
			Bit eq_cat = group_by[i].equal(categories[j]);
			Integer result_sum = zero.select(eq_cat, values[i]);

			sums[j] = sums[j] + result_sum;
			emp::add_full(counts[j].bits.data(), nullptr, counts[j].bits.data(), zero.bits.data(), &eq_cat, counts[j].size());	// counts[j] = counts[j] + eq_cat
		}	
	}

    for (int i = 0; i < cat_len; ++i) {
		int sum = sums[i].reveal<int>();
		int count = counts[i].reveal<int>();
		float average = (float) sum / count;
        cout << "Average (" << i << "): " << average << endl;
	}

	delete[] group_by;
	delete[] values;
}


void test_average2_fast(int party, vector<vector<string>> inputs, char* agg_cols, char* value_col, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size * 2];		//  May contain inputs of both parties
	Integer *values = new Integer[sample_size];
	
	Integer sums[first_cat_len][second_cat_len];
	Integer counts[first_cat_len][second_cat_len];
	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, inputs, agg_cols);
	initialize_values_i(party, values, inputs, value_col);

	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			sums[i][j] = Integer(BITSIZE, 0, PUBLIC);
			counts[i][j] = Integer(BITSIZE, 0, PUBLIC);
		}
	}

	for (int i = 0; i < first_cat_len; ++i) {
		categories_1[i] = Integer(BITSIZE, i, PUBLIC);
	}

	for (int i = 0; i < second_cat_len; ++i) {
		categories_2[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[sample_size + i].equal(categories_2[k]);
				Bit match = eq_first_cat & eq_second_cat;
				Integer result_sum = zero.select(match, values[i]);

				sums[j][k] = sums[j][k] + result_sum;
				emp::add_full(counts[j][k].bits.data(), nullptr, counts[j][k].bits.data(), zero.bits.data(), &match, counts[j][k].size());	// counts[j][k] = counts[j][k] + match

			}	
		}	
	}
	
	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			int sum = sums[i][j].reveal<int>();
			int count = counts[i][j].reveal<int>();
			float average = (float) sum / count;
			cout << "Average (" << i << ", " << j << "): " << average << endl;
		}
	}

   delete[] group_by;
   delete[] values;
}


void test_mode(int party, vector<vector<string>> inputs, char* agg_cols, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size * 2];		//  May contain inputs of both parties

	Integer frequencies [first_cat_len][second_cat_len];
	Integer modes[first_cat_len];

	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, inputs, agg_cols);

	for(int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j)
			frequencies[i][j] = Integer(BITSIZE, 0);
	}

	for(int i = 0; i < first_cat_len; ++i) 
		categories_1[i] = Integer(BITSIZE, i, PUBLIC);

	
	for (int i = 0; i < second_cat_len; ++i)
		categories_2[i] = Integer(BITSIZE, i, PUBLIC);

	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[sample_size + i].equal(categories_2[k]);
				Bit match = eq_first_cat & eq_second_cat;

				emp::add_full(frequencies[j][k].bits.data(), nullptr, frequencies[j][k].bits.data(), zero.bits.data(), &match, frequencies[j][k].size());	// counts[j][k] = counts[j][k] + match
			}	
		}	
	}

	// With the frequencies calculated, find the mode for each group
	for (int i = 0; i < first_cat_len; ++i) {
		Integer max(BITSIZE, 0);
		Integer mode(BITSIZE, -1);
		for (int j = 0; j < second_cat_len; ++j) {
			Integer freq = frequencies[i][j];
			Bit geq = freq.geq(max);

			// This will only update max when freq is greater or equal to max
			max = max.select(geq, freq);

			// If max was assigned a new value, we need to update the mode for the group
			Bit eq_max = freq.equal(max);
			mode = mode.select(eq_max, categories_2[j]);
		}
		modes[i] = mode;		
	}

	// Frequencies are not revealed, only the mode
	for (int i = 0; i < first_cat_len; ++i) {
		cout << "Group " << i << endl;
		//for (int j = 0; j < second_cat_len; ++j) {
		//	cout <<  "Frequency of the value " << j << ": " << frequencies[i][j].reveal<int>() << endl;
		//}
		cout << "Mode: " << modes[i].reveal<int>() << endl;
	}

	delete[] group_by;
}


void test_freq(int party, vector<vector<string>> inputs, char* agg_cols, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	int sample_size = inputs[0].size();
	Integer *group_by = new Integer[sample_size * 2];		//  May contain inputs of both parties

	Integer frequencies [first_cat_len][second_cat_len];
	Integer categories_1 [first_cat_len];
	Integer categories_2 [second_cat_len];

	initialize_groupby_inputs(party, group_by, inputs, agg_cols);

	for(int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j)
			frequencies[i][j] = Integer(BITSIZE, 0);
	}

	for(int i = 0; i < first_cat_len; ++i) 
		categories_1[i] = Integer(BITSIZE, i, PUBLIC);

	
	for (int i = 0; i < second_cat_len; ++i)
		categories_2[i] = Integer(BITSIZE, i, PUBLIC);

	// Calculate frequencies of each item by group
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < sample_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[sample_size + i].equal(categories_2[k]);
				Bit match = eq_first_cat & eq_second_cat;

				emp::add_full(frequencies[j][k].bits.data(), nullptr, frequencies[j][k].bits.data(), zero.bits.data(), &match, frequencies[j][k].size());	// counts[j][k] = counts[j][k] + match
			}	
		}	
	}

	for (int i = 0; i < first_cat_len; ++i) {
		cout << "Group " << i << endl;
		for (int j = 0; j < second_cat_len; ++j) {
			cout <<  "Frequency of the value " << j << ": " << frequencies[i][j].reveal<int>() << endl;
		}
	}

	delete[] group_by;
}


void xtabs_1(int party, vector<vector<string>> inputs, char aggregation, char* agg_cols, char* value_col) {
	switch (aggregation) {
		case 's':
			utils::time_it(test_sum1, party, inputs, agg_cols, value_col, CAT_LEN);
			break;
		case 'a':
			//utils::time_it(test_average1, party, inputs, agg_cols, value_col, CAT_LEN);
			utils::time_it(test_average1_fast, party, inputs, agg_cols, value_col, CAT_LEN);
			break;
		case 'm':
			cout << "Mode is not available when grouping by one column only" << endl;
			break;
		case 'f':
			cout << "Frequency counts are not available when grouping by one column only" << endl;
			break;
		case 'd':
			cout << "Standard Deviation" << endl;
			cout << "NOT IMPLEMENTED" << endl;
			break;
		default:
			cout << "Invalid aggregation type" << endl;
			break;
	}

}

void xtabs_2(int party, vector<vector<string>> inputs, char aggregation, char* agg_cols, char* value_col) {
	switch (aggregation) {
		case 's':
			utils::time_it(test_sum2, party, inputs, agg_cols, value_col, CAT_LEN, CAT_LEN);
			break;
		case 'a':
			//utils::time_it(test_average2, party, inputs, agg_cols, value_col, CAT_LEN, CAT_LEN);
			utils::time_it(test_average2_fast, party, inputs, agg_cols, value_col, CAT_LEN, CAT_LEN);
			break;
		case 'm':
			utils::time_it(test_mode, party, inputs, agg_cols, CAT_LEN, CAT_LEN);
			break;
		case 'f':
			utils::time_it(test_freq, party, inputs, agg_cols, CAT_LEN, CAT_LEN);
			break;
		case 'd':
			cout << "Standard Deviation" << endl;
			cout << "NOT IMPLEMENTED" << endl;
			break;
		default:
			cout << "Invalid aggregation type" << endl;
			break;
	}
}

// Right now this requires setting a value column for mode and frequency counts even though they are not used
void test_xtabs(int party, vector<vector<string>> inputs, char aggregation, char* agg_cols, char* value_col) {
	int num_agg_cols = strlen(agg_cols) / 2;	// Number of aggregation columns (e.g. a0b1 -> 2)

	if (num_agg_cols == 1) {
		xtabs_1(party, inputs, aggregation, agg_cols, value_col);
	}
	else if (num_agg_cols == 2) {
		xtabs_2(party, inputs, aggregation, agg_cols, value_col);
	}
	else {
		cout << "Invalid number of aggregation columns" << endl;
		return;
	}
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

	vector<vector<string>> input_matrix;	// Dataset, essentially

	DIR *dir;
	struct dirent *ent;
	// Reads the files in order and pushes the contents to the input_matrix. As such, the order of the files is important (which the generator script handles)
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
		delete io;
		return EXIT_FAILURE;
	}

	// Assert that all files have the same number of elements
	for (size_t i = 1; i < input_matrix.size(); ++i) {
		if (input_matrix[i].size() != input_matrix[0].size()) {
			cerr << "Error: All files must have the same number of elements." << endl;
			delete io;
			return 1;
		}
	}

	cout << "Number of files read: " << input_matrix.size() << endl;
	cout << "Number of elements in each file: " << input_matrix[0].size() << endl;
	test_xtabs(party, input_matrix, aggregation[0], agg_cols, value_col);

	delete io;
    return 0;
}
