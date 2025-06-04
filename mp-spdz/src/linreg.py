from Compiler.types import Array, Matrix, sfix
from Compiler.library import print_ln, for_range_opt, for_range, for_range_parallel
from Compiler.compilerLib import Compiler


usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)
compiler.parser.add_option("--rows", dest="rows")
#compiler.parser.add_option("--columns", dest="columns")
compiler.parse_args()
if not compiler.options.rows:
    compiler.parser.error("--rows required")
#if not compiler.options.columns:
#    compiler.parser.error("--columns required")


def simple_linreg():
    """
    Simple linreg where Alice holds the feature column and Bob holds the target column.

    Parameters:
    scale : bool
        Whether to scale the feature column (mean and std normalization). Ideally, inputs are already standardized to avoid 
    """
    
    compiler.prog.use_trunc_pr = True
    sfix.set_precision(16, 47) # 32 integer bits (31 + sign bit (not counted)), 16 fractional bits. Requires compiling with -R 192
    #sfix.set_precision(16, 79)  # 64 integer bits (63 + sign bit (not counted)), 16 fractional bits. Requires compiling with -R 256

    max_rows = int(compiler.options.rows)

    alice = Array(max_rows, sfix)
    bob = Array(max_rows, sfix)

    @for_range_opt(max_rows)
    def _(i):
        alice[i] = sfix.get_input_from(0)
        bob[i] = sfix.get_input_from(1)

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

    print_ln("Sum of X: %s", sum_x.reveal())
    print_ln("Sum of Y: %s", sum_y.reveal())
    print_ln("Sum of XY: %s", sum_xy.reveal())
    print_ln("Sum of X^2: %s", sum_x2.reveal())

    print_ln("Intercept (beta_0): %s", beta_0.reveal())
    print_ln("Slope (beta_1): %s", beta_1.reveal())


@compiler.register_function('linreg')
def main():
    if "simple" in compiler.prog.args:
        print("-----------------------------------------")
        print("Compiling for simple linear regression")
        print("-----------------------------------------")
        simple_linreg()
    elif "sgd" in compiler.prog.args:
        print("-----------------------------------------")
        print("Compiling for linear regression using SGD")
        print("SGD linear regression is not implemented yet.")
        print("-----------------------------------------")
    else:
        print("-----------------------------------------")
        print("Compiling for regular linear regression")
        print("-----------------------------------------")
    

if __name__ == "__main__":
    compiler.compile_func()