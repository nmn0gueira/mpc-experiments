from Compiler.types import Array, Matrix, sfix, sint, cint, cfix
from Compiler.GC.types import sbitintvec, sbitfixvec
from Compiler.library import print_ln, for_range_opt
from Compiler.compilerLib import Compiler

import pandas as pd


usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)

# Options for defining the input matrices and their dimensions
compiler.parser.add_option("--rows", dest="rows", type=int, help="Number of rows for the inputs)")
compiler.parse_args()

if not compiler.options.rows:
    compiler.parser.error("--rows required")


def get_bin_edges(values, value_type):
    num_edges = len(values)
    bin_edges = Array(num_edges, value_type)

    previous = values[0]
    bin_edges[0] = value_type(previous.item())
    
    for i in range(1, num_edges):
        if (values[i] <= previous):
            raise ValueError("Bin edges are not in ascending order")
        bin_edges[i] = value_type(values[i].item())

    return bin_edges, num_edges


# For some reason using a regular for loop in this function seems to just break the for_range_opt optimization in hist2d making it the same as this version but without setting the budget.
def digitize(val, bins, bin_edges, num_edges, secret_type):
    found_index = secret_type(0)
    bin_to_index = secret_type(0)
    
    @for_range_opt(1, num_edges)
    def _(i):
    #for i in range(1, num_edges):  
        leq = val <= bin_edges[i]
        
        select = mux(found_index.bit_not(), leq, ZERO)
        bin_to_index.update(mux(select, bins[i-1], bin_to_index))

        # Only updates found index the first time
        found_index.update(mux(found_index.bit_not(), select, found_index))

    return bin_to_index


def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)


def hist_2d_arithmetic(max_rows, edges_df, types, binary):
    secret_type, clear_type = types

    bin_edges_x, num_edges_x = get_bin_edges(edges_df.iloc[:, 0].values, clear_type)
    bin_edges_y, num_edges_y = get_bin_edges(edges_df.iloc[:, 1].values, clear_type)

    num_bins_x = num_edges_x - 1
    num_bins_y = num_edges_y - 1

    alice = Array(max_rows, secret_type)
    bob = Array(max_rows, secret_type)

    alice.input_from(0, binary=binary)
    bob.input_from(1, binary=binary)
    
    hist2d = Matrix(num_bins_y, num_bins_x, sint)
    hist2d.assign_all(0)

    bins_x = Array(num_bins_x, sint)
    for i in range(num_bins_x):
        bins_x[i] = sint(i)

    bins_y = Array(num_bins_y, sint)
    for i in range(num_bins_y):
        bins_y[i] = sint(i)

    @for_range_opt(max_rows)
    def _(i):
        x_val = alice[i]
        y_val = bob[i]

        bin_index_x = digitize(x_val, bins_x, bin_edges_x, num_edges_x, sint)
        bin_index_y = digitize(y_val, bins_y, bin_edges_y, num_edges_y, sint)

        for y in range(num_bins_y):
            for x in range(num_bins_x):
                hist2d[y][x] += mux(
                    bin_index_x.equal(bins_x[x]).bit_and(bin_index_y.equal(bins_y[y])),
                    ONE,
                    ZERO
                )

    # Reveal the histogram
    print_ln("Histogram 2D:")
    for i in range(num_bins_y):
        for j in range(num_bins_x):
            print_ln("hist2d[%s][%s]: %s", i, j, hist2d[i][j].reveal())



def hist_2d_bin(max_rows, edges_df, secret_type, binary):
    # In binary circuits the only clear type is cbits which does not serve very well for our purposes
    bin_edges_x, num_edges_x = get_bin_edges(edges_df.iloc[:, 0].values, secret_type)
    bin_edges_y, num_edges_y = get_bin_edges(edges_df.iloc[:, 1].values, secret_type)

    num_bins_x = num_edges_x - 1
    num_bins_y = num_edges_y - 1

    alice = Array(max_rows, secret_type)
    bob = Array(max_rows, secret_type)

    alice.input_from(0)
    bob.input_from(1)
    
    hist2d = Matrix(num_bins_y, num_bins_x, SIV32)
    hist2d.assign_all(0)

    bins_x = Array(num_bins_x, SIV32)
    for i in range(num_bins_x):
        bins_x[i] = SIV32(i)

    bins_y = Array(num_bins_y, SIV32)
    for i in range(num_bins_y):
        bins_y[i] = SIV32(i)

    @for_range_opt(max_rows)
    def _(i):
        x_val = alice[i]
        y_val = bob[i]

        bin_index_x = digitize(x_val, bins_x, bin_edges_x, num_edges_x, SIV32)
        bin_index_y = digitize(y_val, bins_y, bin_edges_y, num_edges_y, SIV32)

        for y in range(num_bins_y):
            for x in range(num_bins_x):
                hist2d[y][x] += mux(
                    bin_index_x.equal(bins_x[x]).bit_and(bin_index_y.equal(bins_y[y])),
                    ONE,
                    ZERO
                )

    # Reveal the histogram
    print_ln("Histogram 2D:")
    for i in range(num_bins_y):
        for j in range(num_bins_x):
            print_ln("hist2d[%s][%s]: %s", i, j, hist2d[i][j].reveal())


@compiler.register_function('hist2d')
def main():
    global ZERO
    global ONE

    fixed = 'fix' in compiler.prog.args
    integer = 'int' in compiler.prog.args
    binary = 'binary' in compiler.prog.args
    max_rows = compiler.options.rows

    edges_df = pd.read_csv('Player-Data/public/data.csv')

    if compiler.prog.options.binary != 0: # If the program is being compiled for binary
        global SIV32
        SIV32 = sbitintvec.get_type(32)
        ZERO = SIV32(0)
        ONE = SIV32(1)
        if fixed:
            print("-----------------------------------------------------------------")
            print("Compiling for 2D Histogram using fixed-point numbers (sbitfixvec)")
            print("-----------------------------------------------------------------")
            hist_2d_bin(max_rows, edges_df, sbitfixvec, binary)

        elif integer:
            print("-------------------------------------------------------------")
            print("Compiling for 2D Histogram using integer numbers (sbitintvec)")
            print("-------------------------------------------------------------")
            hist_2d_bin(max_rows, edges_df, SIV32, binary)

    else:
        ZERO = sint(0)  # TODO Change to cint if there is no leakage for slightly more performance and slightly less comm.
        ONE = sint(1)
        if fixed:
            compiler.prog.use_trunc_pr = True
            print("-----------------------------------------------------------")
            print("Compiling for 2D Histogram using fixed-point numbers (sfix)")
            print("-----------------------------------------------------------")
            hist_2d_arithmetic(max_rows, edges_df, (sfix, cfix), binary)

        elif integer:
            print("-------------------------------------------------------")
            print("Compiling for 2D Histogram using integer numbers (sint)")
            print("-------------------------------------------------------")
            hist_2d_arithmetic(max_rows, edges_df, (sint, cint), binary)
    

if __name__ == "__main__":
    compiler.compile_func()