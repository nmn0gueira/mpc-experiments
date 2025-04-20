from Compiler.library import print_ln, for_range_opt, for_range, for_range_parallel
from Compiler.compilerLib import Compiler
from Compiler.GC.types import sbits, sbitintvec, sbitvec
from Compiler.types import Array

CAT_LEN = 4
BITSIZE = 32

def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)
compiler.parser.add_option("--rows", dest="rows")
#compiler.parser.add_option("--columns", dest="columns")
compiler.parse_args()
if not compiler.options.rows:
    compiler.parser.error("--rows required")
#if not compiler.options.columns:
#    compiler.parser.error("--columns required")


@compiler.register_function('xtabs-gc')
def main():
    max_rows = int(compiler.options.rows)

    siv32 = sbitintvec.get_type(BITSIZE)

    a = Array(max_rows, siv32)
    b = Array(max_rows, siv32)

    for i in range(max_rows):
        a[i] = siv32.get_input_from(0)
        b[i] = siv32.get_input_from(1)

    sums = Array(CAT_LEN, siv32)
    categories = Array(CAT_LEN, siv32)

    for i in range(CAT_LEN):
        sums[i] = siv32(0)
        categories[i] = siv32(i)

    @for_range_opt([max_rows, CAT_LEN]) # This is a nested loop
    def _(i, j):
        sums[j] = mux(a[i] == categories[j], sums[j] + b[i], sums[j])

    for i in range(CAT_LEN):
        print_ln("Category %s: %s", i, sums[i].reveal())

if __name__ == "__main__":
    compiler.compile_func()