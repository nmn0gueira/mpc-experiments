from Compiler.types import sfix, Array, Matrix
from Compiler.library import print_ln, for_range_opt, for_range, for_range_parallel
from Compiler.compilerLib import Compiler

import pandas as pd
from sklearn.model_selection import train_test_split


compiler = Compiler()


@compiler.register_function('test')
def main():
    df = pd.read_csv('../data/data.csv').values

    #print(df)
    if 'party0' in compiler.prog.args:
        a = sfix.input_tensor_via(0, df)
        b = sfix.input_tensor_via(1, shape=df.shape)
    elif 'party1' in compiler.prog.args:
        a = sfix.input_tensor_via(0, shape=df.shape)
        b = sfix.input_tensor_via(1, df)
    else:
        a = sfix.input_tensor_via(0, df)
        b = sfix.input_tensor_via(1, df)
        print_ln("Tensor a: %s", a.reveal())
        print_ln("Tensor b: %s", b.reveal())
        return
    

if __name__ == "__main__":
    compiler.compile_func()