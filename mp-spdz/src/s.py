from Compiler.library import print_ln, for_range_opt, for_range, for_range_parallel
from Compiler.compilerLib import Compiler
from Compiler.GC.types import sbits, sbitintvec, sbitvec

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

    siv32 = sbitintvec.get_type(max_rows)

    sb32 = sbits.get_type(BITSIZE)
    sums = siv32([sb32(0) for _ in range(CAT_LEN)]).elements()
    categories = siv32([sb32(i) for i in range(CAT_LEN)]).elements()

    for i in range(max_rows):
        a = sbits.get_input_from(0, BITSIZE)
        b = sbits.get_input_from(1, BITSIZE)
        for j in range(CAT_LEN):
            sums[j] = mux(a == categories[j], sums[j] + b, sums[j])


    for i in range(CAT_LEN):
       print_ln("Category %s: %s", i, sums[i].reveal())

if __name__ == "__main__":
    compiler.compile_func()