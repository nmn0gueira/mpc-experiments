# This file is used to generate MP-SPDZ input from csv files
from Compiler.types import sfix, sint
from Compiler.compilerLib import Compiler

import pandas as pd
from sklearn.model_selection import train_test_split

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)
compiler.parser.add_option("--test_size", dest="test_size", default=0.2, type=float, help="Proportion of the dataset to include in the test split (default: 0.2)")
compiler.parser.add_option("--random_state", dest="random_state", default=42, type=int, help="Random seed for reproducibility (default: 42)")
compiler.parse_args()

# TODO
# Also add option for specifying columns of party 0 and party 1 if not necessary to use all columns
# Maybe think of adding a way to store this input in other places for reuse between using other programs if it turns out to be useful
# Add an option to scale features and normalize labels in here, if necessary.


def csv2spdz(path, party, split, binary):
    """
    Reads a CSV file and inputs the data into MP-SPDZ for the specified party.
    
    Parameters:
    path : str
        Path to the CSV file.
    party : int
        Party ID (0 or 1).
    split : bool
        Whether to split the data into training and test sets.
    binary : bool
        Whether to input the data as binary.
    """
    df = pd.read_csv(path)
    
    if split:
        df_train, df_test = train_test_split(df, test_size=compiler.options.test_size, random_state=compiler.options.random_state) # Use shuffle=False if debugging
        # Store each column as a separate tensor (this helps optimize memory when performing linear regression)
        for col in df_train.columns:
            sint.input_tensor_via(party, df_train[col].values, binary=binary)
            sint.input_tensor_via(party, df_test[col].values, binary=binary)

        print(f"Input data for party {party}: {df_train.shape[0]} training samples, {df_test.shape[0]} test samples.")
    else:
        sint.input_tensor_via(party, df.values, binary=binary)
        print(f"Input data for party {party}: {df.shape[0]} samples.")


@compiler.register_function('csv2spdz')
def main():
    split = 'linreg' in compiler.prog.args  # kind of a hack but works for now as linreg is the only program that needs data split and input in a specific way
    binary = 'binary' in compiler.prog.args
    if 'party0' in compiler.prog.args:
        csv2spdz('Player-Data/alice/data.csv', 0, split, binary)
    if 'party1' in compiler.prog.args:
        csv2spdz('Player-Data/bob/data.csv', 1, split, binary)
        

if __name__ == "__main__":
    compiler.compile_func()