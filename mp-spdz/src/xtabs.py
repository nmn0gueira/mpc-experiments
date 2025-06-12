from Compiler.library import print_ln, for_range_opt
from Compiler.compilerLib import Compiler
from Compiler.types import sint, sfix, Matrix, Array

INPUT_SIZE = 10000
CAT_LEN = 4

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)

# Options for defining the input matrices and their dimensions
compiler.parser.add_option("--rows", dest="rows", type=int, help="Number of rows for the input matrices)")

compiler.parser.add_option("--aggregation", dest="aggregation", type=str, help="Type of aggregation to be performed (sum, average, freq(uencies), st(d)ev)")
compiler.parser.add_option("--group_by", dest="group_by", type=str, help="Columns to group by (2 max) (e.g ab for Alice's first column and Bob's first column")
compiler.parser.add_option("--value_col", dest="label", type=str, help="Value column (not needed for mode and freq.) (e.g b for Bob's column)")

compiler.parse_args()

function_name = f"xtabs-{compiler.options.aggregation}-{len(compiler.options.group_by)}"    # e.g. xtabs-sum-2

def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)


def xtabs_sum():
    max_rows = compiler.options.rows

    alice = Matrix(INPUT_SIZE, 2, sint)
    bob = Matrix(INPUT_SIZE, 2, sint)

    alice.input_from(0)
    bob.input_from(1)

    sums = Array(CAT_LEN, sint)
    categories = Array(CAT_LEN, sint)

    for i in range(CAT_LEN):
        sums[i] = sint(0)
        categories[i] = sint(i)

    @for_range_opt(INPUT_SIZE)
    def _(i):
        @for_range_opt(CAT_LEN)
        def _(j):
            sums[j] = mux(alice[i][0] == categories[j], sums[j] + bob[i][1], sums[j])

    
    for i in range(CAT_LEN):
        print_ln("Sum %s: %s", i, sums[i].reveal())


@compiler.register_function(function_name)
def main():
    xtabs_sum()
    


if __name__ == "__main__":
    compiler.compile_func()