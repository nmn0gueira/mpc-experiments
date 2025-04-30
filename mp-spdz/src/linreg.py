from Compiler.types import sfloat, Array, Matrix, sfix
from Compiler.library import print_ln, for_range_opt, for_range, for_range_parallel
from Compiler.compilerLib import Compiler

CAT_LEN = 4

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)
compiler.parser.add_option("--rows", dest="rows")
#compiler.parser.add_option("--columns", dest="columns")
compiler.parse_args()
if not compiler.options.rows:
    compiler.parser.error("--rows required")
#if not compiler.options.columns:
#    compiler.parser.error("--columns required")


@compiler.register_function('linreg')
def main():
    compiler.prog.use_trunc_pr = True
    max_rows = int(compiler.options.rows)

    alice = Matrix(max_rows, 1, sfix)

    bob = Matrix(max_rows, 1, sfix)

    #@for_range(input_size)
    @for_range_parallel(2, max_rows)
    def _(i):
        alice[i][0] = sfix.get_input_from(0)
        bob[i][0] = sfix.get_input_from(1)

    sum_x = sfix(0)
    sum_y = sfix(0)
    sum_xy = sfix(0)
    sum_x2 = sfix(0)

    @for_range_opt(max_rows)
    def _(i):
        sum_x.update(sum_x + alice[i][0])
        sum_y.update(sum_y + alice[i][0])
        sum_xy.update(sum_xy + alice[i][0] * bob[i][0])
        sum_x2.update(sum_x2 + alice[i][0] ** 2)

    beta_1 = (max_rows * sum_xy - sum_x * sum_y) / (max_rows * sum_x2 - sum_x * sum_x);
    beta_0 = (sum_y - beta_1 * sum_x) / max_rows;

    print_ln("Intercept (beta_0): %s", beta_0.reveal())
    print_ln("Slope (beta_1): %s", beta_1.reveal())

if __name__ == "__main__":
    compiler.compile_func()