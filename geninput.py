import argparse, random, os, math, errno
import numpy as np
from operator import itemgetter

def create_dirs(program):
    dirname = "data/"+program
    if not os.path.exists(dirname):
        try:
            os.makedirs(dirname)
        except OSError as e:
            if e.errno != errno.EEXIST: raise

def get_rand_list(bits, l):
    return [random.getrandbits(bits) for _ in range(l)]


def gen_input(program, n, l):
    if (n > 32):
        print ("invalid bit length---this test can only handle up to 32 bits")
        print ("because we read in input using `stoi`")
        return

    bits = int((n - int(math.log(l, 2))) / 2)
    
    lists = [(i,get_rand_list(bits,l)) for i in [1,2]]
    for party,data in lists:
        with open("data/%s/%d.%s.dat"%(program,n,party),'w') as f:
            for x in data:
                f.write("%d\n"%x)
    print("expected value: %d"%(sum(x*y for x,y in zip(lists[0][1], lists[1][1]))))


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

    bits = int((n - int(math.log(l, 2))) / 2)

    # Bins/categories must have some repeated items
    bins_a = get_rand_list(2, l)
    bins_b = get_rand_list(2, l) # Second bins data file for when computing regular cross tabulation with Bob

    values = get_rand_list(bits, l)


    for party, data in zip([1, 2], [bins_a, bins_b]):
        with open(f"data/xtabs/{n}.bins.{party}.dat",'w') as f:
            for x in data:
                f.write(f"{x}\n")
    
    with open (f"data/xtabs/{n}.vals.2.dat", 'w') as f:
        for x in values:
            f.write(f"{x}\n")

    # Expected values by function
    sums = {}
    for i in range(l):
        sums[bins_a[i]] = sums.get(bins_a[i], 0) + values[i]

    averages = {}
    for i in range(l):
        averages[bins_a[i]] = averages.get(bins_a[i], 0) + values[i]
    for key in averages:
        averages[key] = averages[key] / bins_a.count(key)

    
    frequencies = {}    # absolute frequencies
    for i in range(l):
        frequency_dict = frequencies.get(bins_a[i], {})
        frequency_dict[bins_b[i]] = frequency_dict.get(bins_b[i], 0) + 1
        frequencies[bins_a[i]] = frequency_dict
    modes = {}
    for k, v in frequencies.items():
        modes[k] = max(v.items(), key=itemgetter(1))[0]
    
    print(f"Expected values (sum): {sorted(sums.items())}\n")
    print(f"Expected values (mean): {sorted(averages.items())}\n")
    print(f"Expected values (mode): {sorted(modes.items())}\n")
    print(f"Expected values (frequencies): {sorted(frequencies.items())}\n")



def gen_linreg_input(n, l):
    '''
    Model: y = beta_0 + beta_1 * x
    '''
    if (n > 32):
        print ("invalid bit length---this test can only handle up to 32 bits")
        print ("because we read in input using `stoi`")
        return

    bits = int((n - int(math.log(l, 2))) / 2)
    
    features = get_rand_list(bits, l)   # x
    labels = get_rand_list(bits, l)     # y

    for party, data in zip([1,2], [features, labels]):
        with open(f"data/linreg/{n}.{party}.dat",'w') as f:
            for x in data:
                f.write(f"{x}\n")

    # Sums
    sum_x = np.sum(features)
    sum_y = np.sum(labels)

    
    sum_xy = sum(x*y for x,y in zip(features,labels))
    sum_x2 = sum(x**2 for x in features)

    # Calculate slope (beta_1)
    beta_1 = (l * sum_xy - sum_x * sum_y) / (l * sum_x2 - sum_x ** 2)

    # Calclate intercept (beta_0)
    beta_0 = (sum_y - beta_1 * sum_x) / l
    
    # Calculate training error
    squared_errors = 0
    
    for x, y_true in zip(features, labels):
        y_pred = beta_0 + beta_1 * x
        squared_errors += (y_true - y_pred) ** 2

    mean_squared_error = squared_errors / l
    print (f"Expected training error of model: {mean_squared_error}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='generates input for emp-toolkit sample programs')
    parser.add_argument('-n', default=32, type=int, 
        help="integer bit length")
    parser.add_argument('-l', default=10, type=int, 
        help="array length")
    programs = ["innerprod", "xtabs", "linreg"]
    parser.add_argument('-e', default="xtabs", choices = programs,
        help="program selection")
    args = parser.parse_args()

    create_dirs(args.e)

    if args.e == "innerprod":
        gen_input(args.e, args.n, args.l)

    elif args.e == "xtabs":
        gen_xtabs_input(args.n, args.l)
    
    elif args.e == "linreg":
        gen_linreg_input(args.n, args.l)