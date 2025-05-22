# This file is used to generate MP-SPDZ input from csv files
from Compiler.types import sfix, sint
from Compiler.compilerLib import Compiler

import pandas as pd

compiler = Compiler()


@compiler.register_function('iprep')
def main():
    if 'party0' in compiler.prog.args:
        df = pd.read_csv('Player-Data/alice/data.csv').values
        sint.input_tensor_via(0, df, binary=False)
        print()
        print("-----------------------------------------------------------------------------")
        print(f"Party 0 input has {df.shape[0]} rows and {df.shape[1]} columns")
        print("-----------------------------------------------------------------------------")
        print()
    if 'party1' in compiler.prog.args:
        df = pd.read_csv('Player-Data/bob/data.csv').values
        sint.input_tensor_via(1, df, binary=False)
        print()
        print("-----------------------------------------------------------------------------")
        print(f"Party 1 input has {df.shape[0]} rows and {df.shape[1]} columns")
        print("-----------------------------------------------------------------------------")
        print()
        

if __name__ == "__main__":
    compiler.compile_func()