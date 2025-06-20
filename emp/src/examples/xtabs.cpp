/**
 * This program requires the mapping of the categorical variables to group by to an integer range for the functions to be executed. After the
 * functions are executed the same mapping can be reversed, of course. All functions in this program use at least one categorical variable to 
 * group by.
 * 
 * Note 1: Although it may be possible to extend the functions to work with a variable amount of categorical variables, the code would be a bit more
 * complex and the performance would be worse. For now, the number of categories is fixed to 4 (0, 1, 2, 3).
 * Note 2: All used columns (which are themselves from different files read) are assumed to have the same number of elements.
 */

#include "../emp/emp-sh2pc/emp-sh2pc/emp-sh2pc.h"
#include "../utils.hpp"


#include <iostream>
#include <dirent.h>
#include <cmath>
#include <unistd.h>
using namespace emp;
using namespace std;

const int CAT_LEN = 4;	// For now, the number of categories is fixed to 4 (0, 1, 2, 3)


void initialize_groupby_inputs(int party, Integer *group_by, int input_size, char* agg_cols) {
	int agg_cols_len = strlen(agg_cols);	// Number of characters in the string (NOT THE ACTUAL NUMBER OF COLUMNS)
	const int STEP = 2;	// Each column is represented by two characters (e.g. a0, b1, etc.)
	char party_char;
	int other_party;
	
	if (party == ALICE) {
		party_char = 'a';
		other_party = BOB;
	}
	else {
		party_char = 'b';
		other_party = ALICE;
	}

	for (int i = 0; i < agg_cols_len; i += STEP) {
		if (agg_cols[i] == party_char) {
			ifstream infile = utils::get_input_file(agg_cols[i + 1]);
			string line;

			for (int j = 0; j < input_size; ++j) {
				int group_by_index = i / STEP * input_size + j;
				getline(infile, line);
				group_by[group_by_index] = Integer(BITSIZE, stoi(line), party);	// Only the respective party will have the input value
			}
		}

		else  {
			for (int j = 0; j < input_size; ++j) {
				int group_by_index = i / STEP * input_size + j;
				group_by[group_by_index] = Integer(BITSIZE, 0, other_party);
			}	
		}
	}
}

void initialize_values(int party, Integer *values, int input_size, char* value_col) {
	char party_char;
	int other_party;
	
	if (party == ALICE) {
		party_char = 'a';
		other_party = BOB;
	}
	else {
		party_char = 'b';
		other_party = ALICE;
	}
	
	if (value_col[0] == party_char) {
		ifstream infile = utils::get_input_file(value_col[1]);
		string line;
		
		for (int j = 0; j < input_size; ++j) {
			getline(infile, line);
			values[j] = Integer(BITSIZE, stoi(line), party);
		}
	}
	else {
		for (int j = 0; j < input_size; ++j) {
			values[j] = Integer(BITSIZE, 0, other_party);
		}	
	}
}

void initialize_values(int party, Float *values, int input_size, char* value_col) {
	char party_char;
	int other_party;
	
	if (party == ALICE) {
		party_char = 'a';
		other_party = BOB;
	}
	else {
		party_char = 'b';
		other_party = ALICE;
	}
	
	if (value_col[0] == party_char) {
		ifstream infile = utils::get_input_file(value_col[1]);
		string line;
		
		for (int j = 0; j < input_size; ++j) {
			getline(infile, line);
			values[j] = Float(stof(line), party);
		}
	}
	else {
		for (int j = 0; j < input_size; ++j) {
			values[j] = Float(0, other_party);
		}	
	}
}

void test_sum1(int party, int input_size, char* agg_cols, char* value_col, int cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size];		//  May contain inputs of both parties
	Integer *values = new Integer[input_size];
	
	Integer sums[cat_len];	// If CAT_LEN was not fixed, each dimension would be initialized to the respective number of categories
	Integer categories[cat_len];
	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

	for (int i = 0; i < cat_len; ++i) {
		sums[i] = Integer(BITSIZE, 0, PUBLIC);
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < input_size; ++i) {
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
void test_sum2(int party, int input_size, char* agg_cols, char* value_col, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size * 2];	//  May contain inputs of both parties
	Integer *values = new Integer[input_size];

	Integer sums[first_cat_len][second_cat_len];
	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

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
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[input_size + i].equal(categories_2[k]);
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
void test_average1(int party, int input_size, char* agg_cols, char* value_col, int cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size];		//  May contain inputs of both parties
	Float *values = new Float[input_size];
	
	Float sums[cat_len];
	Float counts [cat_len];
	Integer categories[cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

	for (int i = 0; i < cat_len; ++i) {
		sums[i] = Float();
		counts[i] = Float();
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Float zero = Float();
	Float one = Float(1, PUBLIC);
	for (int i = 0; i < input_size; ++i) {
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



void test_average2(int party, int input_size, char* agg_cols, char* value_col, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size * 2];		//  May contain inputs of both parties
	Float *values = new Float[input_size];
	
	Float sums[first_cat_len][second_cat_len];
	Float counts[first_cat_len][second_cat_len];
	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

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
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[input_size + i].equal(categories_2[k]);
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


void test_average1_fast(int party, int input_size, char* agg_cols, char* value_col, int cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size];		//  May contain inputs of both parties
	Integer *values = new Integer[input_size];
	
	Integer sums[cat_len];
	Integer counts [cat_len];
	Integer categories[cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

	for (int i = 0; i < cat_len; ++i) {
		sums[i] = Integer(BITSIZE, 0, PUBLIC);
		counts[i] = Integer(BITSIZE, 0, PUBLIC);
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Integer zero(BITSIZE, 0);
	for (int i = 0; i < input_size; ++i) {
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


void test_average2_fast(int party, int input_size, char* agg_cols, char* value_col, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size * 2];		//  May contain inputs of both parties
	Integer *values = new Integer[input_size];
	
	Integer sums[first_cat_len][second_cat_len];
	Integer counts[first_cat_len][second_cat_len];
	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

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
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[input_size + i].equal(categories_2[k]);
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


void test_mode(int party, int input_size, char* agg_cols, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size * 2];		//  May contain inputs of both parties

	Integer frequencies [first_cat_len][second_cat_len];
	Integer modes[first_cat_len];

	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);

	for(int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j)
			frequencies[i][j] = Integer(BITSIZE, 0);
	}

	for(int i = 0; i < first_cat_len; ++i) 
		categories_1[i] = Integer(BITSIZE, i, PUBLIC);

	
	for (int i = 0; i < second_cat_len; ++i)
		categories_2[i] = Integer(BITSIZE, i, PUBLIC);

	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[input_size + i].equal(categories_2[k]);
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


void test_freq(int party, int input_size, char* agg_cols, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN) {
	Integer *group_by = new Integer[input_size * 2];		//  May contain inputs of both parties

	Integer frequencies [first_cat_len][second_cat_len];
	Integer categories_1 [first_cat_len];
	Integer categories_2 [second_cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);

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
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[input_size + i].equal(categories_2[k]);
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

void test_std1(int party, int input_size, char* agg_cols, char* value_col, int cat_len=CAT_LEN, int ddof=0) {
	Integer *group_by = new Integer[input_size];		//  May contain inputs of both parties
	Float *values = new Float[input_size];
	
	Float sums[cat_len];
	Float counts [cat_len];
	Float averages[cat_len];
	Float variances[cat_len];
	Integer categories[cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

	for (int i = 0; i < cat_len; ++i) {
		sums[i] = Float();
		counts[i] = Float();
		averages[i] = Float();
		variances[i] = Float();
		categories[i] = Integer(BITSIZE, i, PUBLIC);
	}

	Float zero = Float();
	Float one = Float(1, PUBLIC);
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < cat_len; ++j) {
			Bit eqcat = group_by[i].equal(categories[j]);
			Float result_sum = zero.If(eqcat, values[i]);
			Float result_count = zero.If(eqcat, one);
		
			sums[j] = sums[j] + result_sum;
			counts[j] = counts[j] + result_count;
		}	
	}
	// Calculate averages
	for (int i = 0; i < cat_len; ++i) {
		averages[i] = (sums[i] / counts[i]);
	}

	// Calculate variances
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < cat_len; ++j) {
			Bit eqcat = group_by[i].equal(categories[j]);
			Float result = zero.If(eqcat, values[i] - averages[j]);

			variances[j] = variances[j] + result.sqr();
		}	
	}

	Float ddof_secure = ddof == 0 ? zero : one;
	for (int i = 0; i < cat_len; ++i) {
		variances[i] = variances[i] / (counts[i] - ddof_secure);
	}

    for (int i = 0; i < cat_len; ++i) {
		float std = sqrt(variances[i].reveal<double>());
        cout << "Standard Deviation (" << i << "): " << std << endl;
	}

	delete[] group_by;
	delete[] values;
}


void test_std2(int party, int input_size, char* agg_cols, char* value_col, int first_cat_len=CAT_LEN, int second_cat_len=CAT_LEN, int ddof=0) {
	Integer *group_by = new Integer[input_size * 2];		//  May contain inputs of both parties
	Float *values = new Float[input_size];
	
	Float sums[first_cat_len][second_cat_len];
	Float counts[first_cat_len][second_cat_len];
	Float averages[first_cat_len][second_cat_len];
	Float variances[first_cat_len][second_cat_len];
	Integer categories_1[first_cat_len];
	Integer categories_2[second_cat_len];

	initialize_groupby_inputs(party, group_by, input_size, agg_cols);
	initialize_values(party, values, input_size, value_col);

	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			sums[i][j] = Float();
			counts[i][j] = Float();
			averages[i][j] = Float();
			variances[i][j] = Float();
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
	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[input_size + i].equal(categories_2[k]);
				Bit match = eq_first_cat & eq_second_cat;
				
				Float result_sum = zero.If(match, values[i]);
				Float result_count = zero.If(match, one);

				sums[j][k] = sums[j][k] + result_sum;
				counts[j][k] = counts[j][k] + result_count;	
			}	
		}	
	}

	// Calculate averages
	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			averages[i][j] = (sums[i][j] / counts[i][j]);
		}
	}

	for (int i = 0; i < input_size; ++i) {
		for (int j = 0; j < first_cat_len; ++j) {
			Bit eq_first_cat = group_by[i].equal(categories_1[j]);
			
			for (int k = 0; k < second_cat_len; ++k) {
				Bit eq_second_cat = group_by[input_size + i].equal(categories_2[k]);
				Bit match = eq_first_cat & eq_second_cat;

				Float result = zero.If(match, values[i] - averages[j][k]);

				variances[j][k] = variances[j][k] + result.sqr();
			}	
		}	
	}

	Float ddof_secure = ddof == 0 ? zero : one;
	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			variances[i][j] = variances[i][j] / (counts[i][j] - ddof_secure);
		}
	}
	
	for (int i = 0; i < first_cat_len; ++i) {
		for (int j = 0; j < second_cat_len; ++j) {
			float std = sqrt(variances[i][j].reveal<double>());
        	cout << "Standard Deviation (" << i << ", " << j << "): " << std << endl;
		}
	}

	delete[] group_by;
	delete[] values;
}



void xtabs_1(int party, int input_size, char aggregation, char* agg_cols, char* value_col) {
	switch (aggregation) {
		case 's':
			utils::time_it(test_sum1, party, input_size, agg_cols, value_col, CAT_LEN);
			break;
		case 'a':
			utils::time_it(test_average1, party, input_size, agg_cols, value_col, CAT_LEN);
			break;
		case 'v':
			utils::time_it(test_average1_fast, party, input_size, agg_cols, value_col, CAT_LEN);
			break;
		case 'm':
			cout << "Mode is not available when grouping by one column only" << endl;
			break;
		case 'f':
			cout << "Frequency counts are not available when grouping by one column only" << endl;
			break;
		case 'd':
			utils::time_it(test_std1, party, input_size, agg_cols, value_col, CAT_LEN, 0);
			break;
		default:
			cout << "Invalid aggregation type" << endl;
			break;
	}

}

void xtabs_2(int party, int input_size, char aggregation, char* agg_cols, char* value_col) {
	switch (aggregation) {
		case 's':
			utils::time_it(test_sum2, party, input_size, agg_cols, value_col, CAT_LEN, CAT_LEN);
			break;
		case 'a':
			utils::time_it(test_average2, party, input_size, agg_cols, value_col, CAT_LEN, CAT_LEN);
			break;
		case 'v':
			utils::time_it(test_average2_fast, party, input_size, agg_cols, value_col, CAT_LEN, CAT_LEN);
			break;
		case 'm':
			utils::time_it(test_mode, party, input_size, agg_cols, CAT_LEN, CAT_LEN);
			break;
		case 'f':
			utils::time_it(test_freq, party, input_size, agg_cols, CAT_LEN, CAT_LEN);
			break;
		case 'd':
			utils::time_it(test_std2, party, input_size, agg_cols, value_col, CAT_LEN, CAT_LEN, 0);
			break;
		default:
			cout << "Invalid aggregation type" << endl;
			break;
	}
}

// Right now this requires setting a value column for mode and frequency counts even though they are not used
void test_xtabs(int party, int input_size, char aggregation, char* agg_cols, char* value_col) {
	int num_agg_cols = strlen(agg_cols) / 2;	// Number of aggregation columns (e.g. a0b1 -> 2)

	if (num_agg_cols == 1) {
		xtabs_1(party, input_size, aggregation, agg_cols, value_col);
	}
	else if (num_agg_cols == 2) {
		xtabs_2(party, input_size, aggregation, agg_cols, value_col);
	}
	else {
		cout << "Invalid number of aggregation columns" << endl;
		return;
	}
}


/**
 * TODO: Change the way columns are parsed to be more flexible and not crash with a column that is represented by more than 1 digit
 */
int main(int argc, char **argv) {
	if (argc != 8 && argc != 9) {
		cout << "Usage for Alice (server): <program> 1 <port> <input_size> <aggregation> <aggregate_by> <value_col> <input_dir>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <input_size> <aggregation> <aggregate_by> <value_col> <input_dir>" << endl;
		cout << endl;
		cout << "Additional argument explanation: " << endl;
		cout << "<input_size> argument is the number of elements that will be read in each file (e.g. 1000)" << endl;
		cout << "<aggregation> argument can be one of the following: " << endl;
		cout << "s - sum, a - average, v - fast average, m - mode, f - frequency counts, d - standard deviation" << endl;
		cout << "<aggregate_by> argument has format of a0b1 for using Alice's column 0 and Bob's column 1 to aggregate by" << endl;
		cout << "<value_col> argument has format of a0 for using Alice's column 0 as the value column" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	
	int input_size = atoi(argv[argc - 5]);
	char* aggregation = argv[argc - 4];
	char* agg_cols = argv[argc - 3];
	char* value_col = argv[argc - 2];
	utils::set_directory(argv[argc - 1]);
	
	HighSpeedNetIO * io = new HighSpeedNetIO(ip, port, port + 1);
	auto ctx = setup_semi_honest(io, party);
	ctx->set_batch_size(1024*1024);

	test_xtabs(party, input_size, aggregation[0], agg_cols, value_col);

	finalize_semi_honest();
	utils::print_io_stats(*io, party);
	delete io;

    return 0;
}
