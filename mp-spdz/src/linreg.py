from Compiler.types import Array, Matrix, sfix
from Compiler.library import print_ln, for_range_opt, for_range, for_range_parallel
from Compiler.compilerLib import Compiler
from Compiler import ml
import re
#import torch.nn as nn

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)

# Options for defining the input matrices and their dimensions
compiler.parser.add_option("--rows", dest="rows", type=int, help="Number of rows for the input matrices)")

# Options for defining X and y (The feature columns will all be taken as input at once and the label column must be the last column of the respective party)
compiler.parser.add_option("--features", dest="features", type=str, help="Feature columns (e.g a3b1 for Alice's first 3 columns and Bob's first column)")
compiler.parser.add_option("--label", dest="label", type=str, help="Label column (e.g b for Bob's column)")
compiler.parser.add_option("--test_size", dest="test_size", default=0.2, type=float, help="Proportion of the dataset to include in the test split (default: 0.2)")

# SGD options
compiler.parser.add_option("--n_epochs", dest="n_epochs", default=100, type=int, help="Number of epochs for SGD linear regression (default: 100)")
compiler.parser.add_option("--batch_size", dest="batch_size", default=1, type=int, help="Batch size for SGD linear regression (default: 1)")
compiler.parser.add_option("--learning_rate", dest="learning_rate", default=0.01, type=float, help="Learning rate for SGD linear regression (default: 0.01)")

compiler.parse_args()

if not compiler.options.rows:
    compiler.parser.error("--rows required")


def simple_linreg():
    """
    Simple linreg where Alice holds the feature column and Bob holds the target column. Purely for demonstration purposes.
    """
    max_rows = compiler.options.rows

    alice = Array(max_rows, sfix)
    bob = Array(max_rows, sfix)

    # Change precision to avoid overflows
    sfix.set_precision(16, 47)  # Needs to compile with -R 192.

    alice.input_from(0)
    bob.input_from(1)

    sum_x = sfix(0)
    sum_y = sfix(0)
    sum_xy = sfix(0)
    sum_x2 = sfix(0)

    @for_range_opt(max_rows)
    def _(i):
        sum_x.update(sum_x + alice[i])
        sum_y.update(sum_y + bob[i])
        sum_xy.update(sum_xy + alice[i] * bob[i])
        sum_x2.update(sum_x2 + alice[i] ** 2)

    beta_1 = (max_rows * sum_xy - sum_x * sum_y) / (max_rows * sum_x2 - sum_x * sum_x)
    beta_0 = (sum_y - beta_1 * sum_x) / max_rows

    #print_ln("Sum of X: %s", sum_x.reveal())
    #print_ln("Sum of Y: %s", sum_y.reveal())
    #print_ln("Sum of XY: %s", sum_xy.reveal())
    #print_ln("Sum of X^2: %s", sum_x2.reveal())

    print_ln("Intercept (beta_0): %s", beta_0.reveal())
    print_ln("Slope (beta_1): %s", beta_1.reveal())


def parse_columns(format_str):
    pattern = r"^a(\d*)b(\d*)$"
    
    # Match the pattern
    match = re.match(pattern, format_str)
    
    if match:
        a_str, b_str = match.groups()
        
        a_columns = int(a_str) if a_str else 0
        b_columns = int(b_str) if b_str else 0
        
        return a_columns, b_columns
    
    else:
        raise ValueError(f"Invalid format: {format_str}")
    

def get_X_y(alice_columns, bob_columns, rows_train, rows_test):
    num_features = alice_columns + bob_columns

    print(f"Number of features for Alice: {alice_columns}")
    print(f"Number of features for Bob: {bob_columns}")

    X_train = Matrix(rows_train, num_features, sfix)
    X_test = Matrix(rows_test, num_features, sfix)

    current_train_column = current_test_column = 0
    for _ in range(alice_columns):
        @for_range_opt(rows_train)
        def _(i):
            X_train[i][current_train_column] = sfix.get_input_from(0)
        
        current_train_column += 1

    for _ in range(bob_columns):
        @for_range_opt(rows_train)
        def _(i):
            X_train[i][current_train_column] = sfix.get_input_from(1)

        current_train_column += 1

    for _ in range(alice_columns):
        @for_range_opt(rows_test)
        def _(i):
            X_test[i][current_test_column] = sfix.get_input_from(0)

        current_test_column += 1

    for _ in range(bob_columns):
        @for_range_opt(rows_test)
        def _(i):
            X_test[i][current_test_column] = sfix.get_input_from(1)

        current_test_column += 1
    
    label_holder = 0 if compiler.options.label == 'a' else 1

    y_train = Array(rows_train, sfix)
    y_test = Array(rows_test, sfix)

    y_train.input_from(label_holder)
    y_test.input_from(label_holder)

    return X_train, X_test, y_train, y_test


# To optimize memory usage, the features argument should specify the required columns from each party in ascending order so each column can be taken as input all at once and avoid
# storing an additional matrix for alice's and bob's values
def sgd_linreg():
    if not compiler.options.features or not compiler.options.label:
        compiler.parser.error("--features and --label required")

    rows_train = int(compiler.options.rows * (1 - compiler.options.test_size))
    rows_test = int(compiler.options.rows * compiler.options.test_size)

    alice_columns, bob_columns = parse_columns(compiler.options.features)

    X_train, X_test, y_train, y_test = get_X_y(alice_columns, bob_columns, rows_train, rows_test)    

    """ for i in range(X_train.shape[0]):
        for j in range(X_train.shape[1]):
            print_ln("X_train[%s][%s]: %s", i, j, X_train[i][j].reveal())

    for i in range(X_test.shape[0]):
        for j in range(X_test.shape[1]):
            print_ln("X_test[%s][%s]: %s", i, j, X_test[i][j].reveal()) """

    #print_ln("y_train: %s", y_train.reveal())
    #print_ln("y_test: %s", y_test.reveal())
    
    
    linear = ml.SGDLinear(compiler.options.n_epochs, compiler.options.batch_size)
    linear.fit(X_train, y_train)

    print_ln('Model Weights: %s', linear.opt.layers[0].W[:].reveal())
    print_ln('Model Bias: %s', linear.opt.layers[0].b.reveal())
    #print_ln('Diff: %s', (linear.predict(X_test) - y_test).reveal())
    # Thetas
    #for theta in linear.opt.thetas:
    #    print_ln('Theta: %s', theta.reveal())


    # Something like this that uses proper torch layers might be needed for implementing polyfeats and multivariate linreg
    # (https://mp-spdz.readthedocs.io/en/latest/machine-learning.html#pytorch-interface)
    # (https://mp-spdz.readthedocs.io/en/latest/machine-learning.html#keras-interface)
    """ net = nn.Sequential(
    nn.Flatten(),
    nn.Linear(28 * 28, 128),
    nn.ReLU(),
    nn.Linear(128, 128),
    nn.ReLU(),
    nn.Linear(128, 10)
    )

    ml.set_n_threads(int(program.args[2]))

    layers = ml.layers_from_torch(net, training_samples.shape, 128)

    optimizer = ml.SGD(layers)
    optimizer.fit(
    training_samples,
    training_labels,
    epochs=int(program.args[1]),
    batch_size=128,
    validation_data=(test_samples, test_labels),
    program=program
    ) """



@compiler.register_function('linreg')
def main():
    compiler.prog.use_trunc_pr = True

    if "simple" in compiler.prog.args:
        print("-----------------------------------------")
        print("Compiling for simple linear regression")
        print("-----------------------------------------")
        simple_linreg()
        return

    print("-----------------------------------------")
    print("Compiling for linear regression using SGD")
    print("-----------------------------------------")
    sgd_linreg()
    

if __name__ == "__main__":
    compiler.compile_func()