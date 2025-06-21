# This file is used to generate MP-SPDZ input from csv files
from Compiler.types import sfix, sint, sfloat
from Compiler.GC.types import sbitintvec, sbitfixvec
from Compiler.compilerLib import Compiler

import pandas as pd
from sklearn.model_selection import train_test_split

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)

# Options for defining the input matrices and their dimensions
compiler.parser.add_option("--type", dest="type", type=str, default='sint', help="Type of the input data (default: sint). This is only really required if inputting binary data as sint works for all raw data.")
compiler.parser.add_option("--columns", dest="columns", type=str, help="Columns to be used")

# Split options
compiler.parser.add_option("--test_size", dest="test_size", default=0.2, type=float, help="Proportion of the dataset to include in the test split (default: 0.2)")
compiler.parser.add_option("--random_state", dest="random_state", default=42, type=int, help="Random seed for reproducibility (default: 42)")
compiler.parse_args()

# TODO
# Also add option for specifying columns of party 0 and party 1 if not necessary to use all columns
# Maybe think of adding a way to store this input in other places for reuse between using other programs if it turns out to be useful
# Add an option to scale features and normalize labels in here, if necessary.


def store_df(df, party, secret_type, binary, by_column):
    if by_column:  # Store each column as a separate tensor (this helps optimize memory when performing linear regression, for example)
        for col in df.columns:
            secret_type.input_tensor_via(party, df[col].values, binary=binary)
    else:
        secret_type.input_tensor_via(party, df.values, binary=binary)

def csv2spdz(path, party, secret_type, columns, split, binary, by_column):
    df = pd.read_csv(path)

    if columns:
        # Convert columns to a list of integers if they are provided as indices
        if isinstance(columns, str):
            columns = [int(col) for col in columns.split(',')]
        df = df.iloc[:, columns]
        print(f"Selected columns for party {party}: {columns}")
    
    if split:
        df_train, df_test = train_test_split(df, test_size=compiler.options.test_size, random_state=compiler.options.random_state) # Use shuffle=False if debugging
        store_df(df_train, party, secret_type, binary, by_column)
        store_df(df_test, party, secret_type, binary, by_column)
        print(f"Input data for party {party}: {df_train.shape[0]} training samples, {df_test.shape[0]} test samples.")
    else:
        store_df(df, party, secret_type, binary, by_column)
        print(f"Input data for party {party}: {df.shape[0]} samples.")

    print(f"Data stored by column: {by_column}, binary: {binary}")
    print(f"Data type: {compiler.options.type}, Columns: {columns if columns else 'all'}")


def parse_type(type_str):
    if type_str == 'sint':
        return sint
    elif type_str == 'sfix':
        return sfix
    elif type_str == 'sfloat':
        return sfloat
    elif type_str == 'sbitintvec':
        return sbitintvec
    elif type_str == 'sbitfixvec':
        return sbitfixvec
    else:
        raise ValueError(f"Unsupported type: {type_str}.")
    

def parse_columns(columns):
    a_columns = []
    b_columns = []
    for i in range(0, len(columns), 2):
        if columns[i] == 'a':
            a_columns.append(int(columns[i + 1]))
        elif columns[i] == 'b':
            b_columns.append(int(columns[i + 1]))
    return a_columns, b_columns


@compiler.register_function('csv2spdz')
def main():
    split = 'split' in compiler.prog.args
    # Specifying binary input requires specifying the required secret type which is a bit of a pain if different columns require different types
    # For now, we use sint with raw data as it works for everything and is simpler
    binary = 'binary' in compiler.prog.args
    by_column = 'by_column' in compiler.prog.args

    secret_type = parse_type(compiler.options.type)
    a_columns, b_columns = parse_columns(compiler.options.columns) if compiler.options.columns else (None, None)

    if 'party0' in compiler.prog.args:
        csv2spdz('Player-Data/alice/data.csv', 0, secret_type, a_columns, split, binary, by_column)
    if 'party1' in compiler.prog.args:
        csv2spdz('Player-Data/bob/data.csv', 1, secret_type, b_columns, split, binary, by_column)
        

if __name__ == "__main__":
    compiler.compile_func()