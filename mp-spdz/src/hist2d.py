from Compiler.types import Array, Matrix, sfix, sint
from Compiler.library import print_ln, for_range_opt, for_range, for_range_parallel
from Compiler.compilerLib import Compiler
from Compiler import ml
#import torch.nn as nn


usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)

# Options for defining the input matrices and their dimensions
compiler.parser.add_option("--rows", dest="rows", type=int, help="Number of rows for the input matrices)")
compiler.parse_args()

if not compiler.options.rows:
    compiler.parser.error("--rows required")

NUM_BINS_X = 5
NUM_BINS_Y = 5

MAX_SFIX = 16383
MIN_SFIX = -16383

def calc_extremities(array):
    max_value = sfix(MIN_SFIX)
    min_value = sfix(MAX_SFIX)

    @for_range_opt(array.shape[0])
    def _(i):
        max_value.update(sfix.max(max_value, array[i]))
        min_value.update(sfix.min(min_value, array[i]))
    
    return min_value, max_value

def linspace(size, min_value, max_value):
    array = Array(size, sfix)
    step = (max_value - min_value) / sfix(size - 1)
    array[0] = min_value
    for i in range(1, size-1):
        array[i] = array[i-1] + step
    array[size-1] = max_value
    return array

def digitize(val, bins, bin_edges, num_edges):
    false_bit = sint(0)
    found_index = sint(0)
    bin_to_index = sint(0)
    @for_range_opt(1, num_edges)
    def _(i):
        leq = val <= bin_edges[i]
        
        select = mux(found_index.bit_not(), leq, false_bit)
        bin_to_index.update(mux(select, bins[i-1], bin_to_index))

        # Only updates found index the first time
        found_index.update(mux(found_index.bit_not(), select, found_index))

    return bin_to_index

def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)

@compiler.register_function('hist2d')
def main():
    compiler.prog.use_trunc_pr = True
    max_rows = compiler.options.rows

    alice = Array(max_rows, sfix)
    bob = Array(max_rows, sfix)

    @for_range_opt(max_rows)
    def _(i):
        alice[i] = sfix.get_input_from(0)
        bob[i] = sfix.get_input_from(1)
    
    hist2d = Matrix(NUM_BINS_Y, NUM_BINS_X, sint)
    hist2d.assign_all(0)

    num_edges_x = NUM_BINS_X + 1
    num_edges_y = NUM_BINS_Y + 1

    bins_x = Array(NUM_BINS_X, sint)
    for i in range(NUM_BINS_X):
        bins_x[i] = sint(i)

    bins_y = Array(NUM_BINS_Y, sint)
    for i in range(NUM_BINS_Y):
        bins_y[i] = sint(i)

    min_x, max_x = calc_extremities(alice)
    min_y, max_y = calc_extremities(bob)

    bin_edges_x = linspace(num_edges_x, min_x, max_x)
    bin_edges_y = linspace(num_edges_y, min_y, max_y)

    @for_range_opt(max_rows)
    def _(i):
        x_val = alice[i]
        y_val = bob[i]

        bin_index_x = digitize(x_val, bins_x, bin_edges_x, num_edges_x)
        bin_index_y = digitize(y_val, bins_y, bin_edges_y, num_edges_y)

        @for_range_opt(NUM_BINS_Y)
        def _(y):
            @for_range_opt(NUM_BINS_X)
            def _(x):
                hist2d[y][x] += mux(
                    bin_index_x.equal(bins_x[x]).bit_and(bin_index_y.equal(bins_y[y])),
                    sint(1),
                    sint(0)
                )

    # Reveal the bin edges
    print_ln("Bin edges X:")
    for i in range(num_edges_x):
        print_ln("bin_edges_x[%s]: %s", i, bin_edges_x[i].reveal())

    print_ln("Bin edges Y:")
    for i in range(num_edges_y):
        print_ln("bin_edges_y[%s]: %s", i, bin_edges_y[i].reveal())

    # Reveal the histogram
    print_ln("Histogram 2D:")
    for i in range(NUM_BINS_X):
        for j in range(NUM_BINS_Y):
            print_ln("hist2d[%s][%s]: %s", i, j, hist2d[i][j].reveal())


    

if __name__ == "__main__":
    compiler.compile_func()