# This file is used to generate MP-SPDZ input from csv files
from Compiler.types import sfix
from Compiler.compilerLib import Compiler

import pandas as pd

compiler = Compiler()


@compiler.register_function('iprep')
def main():
    if 'party0' in compiler.prog.args:
        df = pd.read_csv('Player-Data/alice/data.csv').values
        sfix.input_tensor_via(0, df, binary=True)
        print("Party 0")

    if 'party1' in compiler.prog.args:
        df = pd.read_csv('Player-Data/bob/data.csv').values
        sfix.input_tensor_via(1, df, binary=True)
        print("Party 1")
        

if __name__ == "__main__":
    compiler.compile_func()