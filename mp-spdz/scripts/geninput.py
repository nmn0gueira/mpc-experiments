import argparse, random, os, math, errno
import numpy as np
import pandas as pd
from operator import itemgetter

np.set_printoptions(legacy='1.25')

BASE_DIR = "data/"
PARTY_ALICE = "alice"
PARTY_BOB= "bob"

def create_dirs(program):
    dirname = BASE_DIR + program + "/"
    alice_dir = dirname + PARTY_ALICE
    bob_dir = dirname + PARTY_BOB
    if not os.path.exists(alice_dir) or not os.path.exists(bob_dir):
        try:
            os.makedirs(alice_dir)
            os.makedirs(bob_dir)
        except OSError as e:
            if e.errno != errno.EEXIST: raise


def get_rand_list(bits, l):
    return [random.getrandbits(bits) for _ in range(l)]


def write_to_csv(program, party, *columns):
    filepath = os.path.join(BASE_DIR, program, party, "data.csv")
    data = {}
    for i, column in enumerate(columns):
        data["col" + str(i)] = column

    pd.DataFrame(data).to_csv(filepath, index=False)


def gen_input(n, l, adjust_bit_length=True):
    '''
    General function for generating the actual values for the data used by the sample programs.

    Parameters:
    n (int): The max number of bits for a number (not accounting for bit adjustment).
    l (int): The generated input size
    adjust_bit_length (bool): If true, the bit length specified will be adjusted to mitigate operations with extremely large
    numbers which may lead to overflows and such. Should be False when working with smaller bit sizes.

    Returns:    
    A tuple of lists, where the first element is the list of numbers for party 1 and the second element is the list of numbers for party 2.
    '''
    if (n > 32):
        raise ValueError("invalid bit length---this test can only handle up to 32 bits")

    bits = n

    if (adjust_bit_length):
        bits = int((n - int(math.log(l, 2))) / 2)
    
    list_a = get_rand_list(bits, l)
    list_b = get_rand_list(bits, l)
    
    return list_a, list_b


def gen_xtabs_input(n, l):
    '''
    Generates input for xtabs program. This input functions as if both parties already have their input ordered. One party has the (typically 
    categorical) value to group by (e.g. education level) and the other has the (typically continuous) values to aggregate upon (e.g. salary). The 
    output will depend on the function that is used to aggregate the values.
    '''

    categories_a, categories_b = gen_input(2, l, adjust_bit_length=False)
    values_a, values_b = gen_input(n, l)

    print_xtabs(categories_a, categories_b, values_b)
    return (categories_a, values_a), (categories_b, values_b)


def print_xtabs(categories_a, categories_b, values):
    input_len = len(categories_a)

    # Sums and averages
    sums = {}
    averages = {}
    counts = {k : categories_a.count(k) for k in set(categories_a)}

    # Frequency counts
    frequencies = {}    # absolute frequencies
    modes = {}

    # Std
    std_values = {}
    std0 = {}   # ddof=0
    std1 = {}   # ddof=1

    for i in range(input_len):
        sums[categories_a[i]] = sums.get(categories_a[i], 0) + values[i]
        std_values[categories_a[i]] = std_values.get(categories_a[i], [])
        std_values[categories_a[i]].append(values[i])

        frequency_dict = frequencies.get(categories_a[i], {})
        frequency_dict[categories_b[i]] = frequency_dict.get(categories_b[i], 0) + 1
        frequencies[categories_a[i]] = frequency_dict

    # Calculate averages
    for key in sums:
        averages[key] = sums[key] / counts[key]
    
    for key in std_values:
        #d2_sum = 0
        #for value in std_values[key]:
        #    d2 = abs(value - averages[key]) ** 2
        #    d2_sum += d2
        #var0 = d2_sum / counts[key]
        #var1 = d2_sum / (counts[key] - 1)
        #std0[key] = var0**0.5
        #std1[key] = var1**0.5
        #print(std0[key])
        #print(np.std(std_values[key], mean=averages[key]))
        #print(std1[key])
        #print(np.std(std_values[key], mean=averages[key], ddof=1))
        std0[key] = np.std(std_values[key], mean=averages[key])
        std1[key] = np.std(std_values[key], mean=averages[key], ddof=1) if len(std_values[key]) > 1 else None

        
    for k, v in frequencies.items():
        modes[k] = max(v.items(), key=itemgetter(1))[0]
        frequencies[k] = dict(sorted(frequencies[k].items()))   # Also sort the frequencies for better readability

    
    print("Grouping by column in a and aggregating on value column b:")
    print(f"Expected values (sum): {sorted(sums.items())}\n")
    print(f"Expected values (avg): {sorted(averages.items())}\n")
    print(f"Expected values (std0): {sorted(std0.items())}\n")
    print(f"Expected values (std1): {sorted(std1.items())}\n")

    print("-----------------------------------------------------------------------")
    print("Grouping by column in a and b (no value column):")
    print(f"Expected values (mode): {sorted(modes.items())}\n")
    print(f"Expected values (frequencies): {sorted(frequencies.items())}\n")

    print("-----------------------------------------------------------------------")

    sums = {}
    averages = {}

    std_values = {}
    std0 = {}   # ddof=0
    std1 = {}   # ddof=1
    
    for i in range(input_len):
        sum_dict = sums.get(categories_a[i], {})
        sum_dict[categories_b[i]] = sum_dict.get(categories_b[i], 0) + values[i]
        sums[categories_a[i]] = sum_dict

        value_dict = std_values.get(categories_a[i], {})
        value_dict[categories_b[i]] = value_dict.get(categories_b[i], [])
        value_dict[categories_b[i]].append(values[i])
        std_values[categories_a[i]] = value_dict

    # Divide by each time a category combo appeared
    for key in sums:
        averages[key] = {}  # Create dictionary
        for k in sums[key]:
            averages[key][k] = sums[key][k] / frequencies[key][k]


    for key in std_values:
        std0[key] = {}
        std1[key] = {}
        for k in std_values[key]:
            std0[key][k] = np.std(std_values[key][k], mean=averages[key][k])
            std1[key][k] = np.std(std_values[key][k], mean=averages[key][k], ddof=1) if len(std_values[key][k]) > 1 else None
    
    for k, v in sums.items():
        sums[k] = dict(sorted(sums[k].items()))
        averages[k] = dict(sorted(averages[k].items())) 
        std0[k] = dict(sorted(std0[k].items()))
        std1[k] = dict(sorted(std1[k].items()))

    print("Grouping by column in a and b and aggregating on value column b:")
    print(f"Expected values (sum): {sorted(sums.items())}\n")
    print(f"Expected values (avg): {sorted(averages.items())}\n")
    print(f"Expected values (std0): {sorted(std0.items())}\n")
    print(f"Expected values (std1): {sorted(std1.items())}\n")


def gen_linreg_input(n, l):
    '''
    Model: y = beta_0 + beta_1 * x
    '''
    
    features, labels = gen_input(n, l)

    print_linreg(features, labels)
    return (features,), (labels,)


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
    values_a, values_b = gen_input(n, l)

    print_hist2d(values_a, values_b)
    return (values_a,), (values_b,)


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
        alice_data, bob_data = program_function(args.n, args.l)
        write_to_csv(args.e, PARTY_ALICE, *alice_data)
        write_to_csv(args.e, PARTY_BOB, *bob_data)

    else:
        print(f"Unknown program: {args.e}") # Should not happen