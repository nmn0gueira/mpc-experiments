import argparse, random, os, math, errno
import numpy as np
from operator import itemgetter

BASE_DIR = "data/"

def create_dirs(program):
    dirname = "data/"+program
    alice_dir = dirname + "/alice"
    bob_dir = dirname + "/bob"
    if not os.path.exists(alice_dir) or not os.path.exists(bob_dir):
        try:
            os.makedirs(alice_dir)
            os.makedirs(bob_dir)
        except OSError as e:
            if e.errno != errno.EEXIST: raise


def get_data_filepath(program, n, party):
    return f"{BASE_DIR}{program}/{party}/{n}.{party}.dat"


def write_to_file(filepath, data):
    with open(filepath, 'w') as f:
        for item in data:
            f.write(f"{item}\n")


# If necessary, change this to account for more than 1D arrays
def get_rand_list(bits, l):
    return [random.getrandbits(bits) for _ in range(l)]


# Right now only does 1 column for each party, maybe change later or remove abstraction
def gen_input(program, n, l, adjust_bit_length=True):
    '''
    General function for generating the actual values for the data used by the sample programs and handling I/O.

    Parameters:
    n (int): The max number of bits for a number (not accounting for bit adjustment).
    l (int): The generated input size
    adjust_bit_length (bool): If true, the bit length specified will be adjusted to mitigate operations with extremely large
    numbers which may lead to overflows and such. Should be False when working with smaller bit sizes.

    Returns:
    A tuple of lists, where the first element is the list of numbers for party 1 and the second element is the list of numbers for party 2.
    '''
    if (n > 32):
        print ("invalid bit length---this test can only handle up to 32 bits")
        print ("because we read in input using `stoi`")
        return

    bits = n

    if (adjust_bit_length):
        bits = int((n - int(math.log(l, 2))) / 2)
    
    list_a = get_rand_list(bits, l)
    list_b = get_rand_list(bits, l)
    
    for party, data in zip(["alice", "bob"], [list_a, list_b]):
        filepath = get_data_filepath(program, n, party)
        write_to_file(filepath, data)
    
    return list_a, list_b


def gen_xtabs_input(n, l):
    '''
    Generates input for xtabs program. This input functions as if both parties already have their input ordered. One party has the (typically 
    categorical) value to group by (e.g. education level) and the other has the (typically continuous) values to aggregate upon (e.g. salary). The 
    output will depend on the function that is used to aggregate the values.
    '''
    if (n > 32):
        print ("invalid bit length---this test can only handle up to 32 bits")
        print ("because we read in input using `stoi`")
        return

    categories_a, categories_b = gen_input('xtabs', 2, l, adjust_bit_length=False)
    _, values_b = gen_input('xtabs', n, l)

    print_xtabs(categories_a, categories_b, values_b)


def print_xtabs(categories_a, categories_b, values):
    # Expected values by function
    sums = {}
    averages = {}
    frequencies = {}    # absolute frequencies
    modes = {}
    input_len = len(categories_a)

    for i in range(input_len):
        sums[categories_a[i]] = sums.get(categories_a[i], 0) + values[i]

        averages[categories_a[i]] = averages.get(categories_a[i], 0) + values[i]

        frequency_dict = frequencies.get(categories_a[i], {})
        frequency_dict[categories_b[i]] = frequency_dict.get(categories_b[i], 0) + 1
        frequencies[categories_a[i]] = frequency_dict

    for key in averages:
        averages[key] = averages[key] / categories_a.count(key)
        
    for k, v in frequencies.items():
        modes[k] = max(v.items(), key=itemgetter(1))[0]
        frequencies[k] = dict(sorted(frequencies[k].items()))   # Also sort the frequencies for better readability
        
    
    print("Grouping by column in a and aggregating on value column b:")
    print(f"Expected values (sum): {sorted(sums.items())}\n")
    print(f"Expected values (mean): {sorted(averages.items())}\n")

    print("-----------------------------------------------------------------------")
    print("Grouping by column in a and b:")
    print(f"Expected values (mode): {sorted(modes.items())}\n")
    print(f"Expected values (frequencies): {sorted(frequencies.items())}\n")

    print("-----------------------------------------------------------------------")

    sums = {}
    averages = {}
    
    for i in range(input_len):
        sum_dict = sums.get(categories_a[i], {})
        sum_dict[categories_b[i]] = sum_dict.get(categories_b[i], 0) + values[i]
        sums[categories_a[i]] = sum_dict

        average_dict = averages.get(categories_a[i], {})
        average_dict[categories_b[i]] = average_dict.get(categories_b[i], 0) + values[i]
        averages[categories_a[i]] = average_dict

    for key in averages:
        for k, v in averages[key].items():
            averages[key][k] = v / categories_a.count(key)
    
    for k, v in sums.items():
        sums[k] = dict(sorted(sums[k].items()))
        averages[k] = dict(sorted(averages[k].items())) 

    print("Grouping by column in a and b and aggregating on value column b:")
    print(f"Expected values (sum): {sorted(sums.items())}\n")
    print(f"Expected values (avg): {sorted(averages.items())}\n")


def gen_linreg_input(n, l):
    '''
    Model: y = beta_0 + beta_1 * x
    '''
    if (n > 32):
        print ("invalid bit length---this test can only handle up to 32 bits")
        print ("because we read in input using `stoi`")
        return

    features, labels = gen_input('linreg', n, l)

    print_linreg(features, labels)


def print_linreg(features, labels, scale=True):
    if (scale):
        mean = np.mean(features)
        std = np.std(features)
        features = (features - mean) / std
        print(f"Expected mean: {mean}")
        print(f"Expected standard deviation: {std}")
 
    sum_x = np.sum(features)
    sum_y = np.sum(labels)
    sum_xy = sum(x*y for x,y in zip(features,labels))
    sum_x2 = sum(x**2 for x in features)
    input_size = len(features)
    
    beta_1 = (input_size * sum_xy - sum_x * sum_y) / (input_size * sum_x2 - sum_x ** 2)
    beta_0 = (sum_y - beta_1 * sum_x) / input_size

    squared_errors = sum((y_true - (beta_0 + beta_1 * x)) ** 2 for x, y_true in zip(features, labels))

    print(f"Expected intercept (beta_0): {beta_0}")
    print(f"Expected slope (beta_1): {beta_1}")
    print(f"Expected training error of model (MSE): {squared_errors / len(features)}")


def gen_hist2d_input(n, l):
    if (n > 32):
        print ("invalid bit length---this test can only handle up to 32 bits")
        print ("because we read in input using `stoi`")
        return

    values_a, values_b = gen_input('hist2d', n, l)

    print_hist2d(values_a, values_b)


def print_hist2d(values_a, values_b):
    NUM_BINS_X = 5
    NUM_BINS_Y = 5
    input_size = len(values_a)

    bin_edges_x = np.linspace(min(values_a), max(values_a), NUM_BINS_X + 1)
    bin_edges_y = np.linspace(min(values_b), max(values_b), NUM_BINS_Y + 1)

    histogram = [[0] * (NUM_BINS_Y) for _ in range(NUM_BINS_X)]
    
    for i in range(input_size):
        x_val = values_a[i]
        y_val = values_b[i]
        
        x_index = 0
        y_index = 0

        # Formula for binning is bin[i-1] < x <= bin[i]
        for x_i in range(1, len(bin_edges_x)):
            if x_val <= bin_edges_x[x_i]:
                x_index = x_i - 1 # bin index
                break
        
        for y_i in range(1, len(bin_edges_y)):
            if y_val <= bin_edges_y[y_i]:
                y_index = y_i - 1 # bin index
                break

        histogram[x_index][y_index] += 1

    print("2D Histogram (Text Representation):")

    # Print the y-axis labels (bin edges)
    print("    ", end="")
    for x_bin in bin_edges_x:
        print(f"{round(x_bin, 2):>5}", end=" ")
    print()
    
    # Print the histogram rows
    for i, row in enumerate(histogram):
        print(f"{round(bin_edges_y[i], 2):>5} ", end="")  # Print x-axis labels (bin edges)
        for count in row:
            print(f"{count:>5}", end=" ")  # Print the counts for each bin
        print()
    print(f"{round(bin_edges_y[-1], 2):>5} ", end="")  # Print x-axis labels (bin edges)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='generates input for emp-toolkit sample programs')
    parser.add_argument('-n', default=32, type=int, 
        help="integer bit length")
    parser.add_argument('-l', default=10, type=int, 
        help="array length")
    
    PROGRAMS = {
        "xtabs": gen_xtabs_input,
        "linreg": gen_linreg_input,
        "hist2d": gen_hist2d_input
    }

    parser.add_argument('-e', default="xtabs", choices = PROGRAMS,
        help="program selection")
    args = parser.parse_args()

    create_dirs(args.e)

    program_function = PROGRAMS.get(args.e)

    if program_function:
        program_function(args.n, args.l)
    else:
        print(f"Unknown program: {args.e}") # Should not happen