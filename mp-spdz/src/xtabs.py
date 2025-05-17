from Compiler.types import sfix, Array, Matrix, sint
from Compiler.library import print_ln, for_range_opt
from Compiler.compilerLib import Compiler

import pandas as pd

CAT_LEN = 4

def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)
compiler.parser.add_option("--party0_cols", dest="party0_cols")
compiler.parser.add_option("--party1_cols", dest="party1_cols")
compiler.parser.add_option("--rows", dest="rows")
compiler.parse_args()


# Later on, use sfix instead of sint for most things probably
@compiler.register_function('xtabs')
def main():
    #print(df)
    if 'party0' in compiler.prog.args:
        if compiler.options.party1_cols is None:
            compiler.parser.error("As party0, --party1_cols required")
        
        party1_cols = int(compiler.options.party1_cols)
        
        df = pd.read_csv('Player-Data/alice/data.csv').values
        a = sint.input_tensor_via(0, df)
        b = sint.input_tensor_via(1, shape=df.shape)
    elif 'party1' in compiler.prog.args:
        if compiler.options.party0_cols is None:
            compiler.parser.error("As party1, --party0_cols required")

        party0_cols = int(compiler.options.party0_cols)
        
        df = pd.read_csv('Player-Data/bob/data.csv').values
        a = sint.input_tensor_via(0, shape=df.shape)
        b = sint.input_tensor_via(1, df)
    else:
        print("No party specified. Compiling as dealer.")
        if compiler.options.party0_cols is None or compiler.options.party1_cols is None or compiler.options.rows is None:
            compiler.parser.error("As dealer, --party0_cols, --party1_cols and --rows required")
        
        party0_cols = int(compiler.options.party0_cols)
        party1_cols = int(compiler.options.party1_cols)
        rows = int(compiler.options.rows)

        a = sint.input_tensor_via(0, shape=(rows, party0_cols))
        b = sint.input_tensor_via(1, shape=(rows, party1_cols))

    # Later on, make it like emp where there is one variable for aggregation columns potentially made of two parties's inputs and one for value cols
    print_ln("Tensor a: %s", a.reveal())
    print_ln("Tensor b: %s", b.reveal())
    print_ln("Tensor a size: %s", a.sizes)
    print_ln("Tensor b size: %s", b.sizes)

    sums = Array(CAT_LEN, sint)
    categories = Array(CAT_LEN, sint)

    for i in range(CAT_LEN):
        categories[i] = sint(i)
        sums[i] = sint(0)
    
    sample_size = a.sizes[0]
    @for_range_opt([sample_size, CAT_LEN]) # This is a nested loop
    def _(i, j):
        sums[j] = mux(a[i][0] == categories[j], sums[j] + b[i][1], sums[j])


    for i in range(CAT_LEN):
        print_ln("Category %s: %s", i, sums[i].reveal())
    

if __name__ == "__main__":
    compiler.compile_func()