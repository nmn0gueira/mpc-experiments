from Compiler.types import sint, Array
from Compiler.library import print_ln, for_range_opt
from Compiler.compilerLib import Compiler

CAT_LEN = 4

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

@compiler.register_function('xtabs-sum')
def main():
    numrows = int(compiler.options.rows)
    rows = range(numrows)

    # For now categories are held by Alice (see about Array class)
    a = [sint.get_input_from(0) for _ in rows]
    # And values by Bob
    b = [sint.get_input_from(1) for _ in rows]

    # We need to create a list of sums for each category
    sums = Array(CAT_LEN, sint)
    sums.assign_all(0)
    
    # Now we can sum the values for each category
    for i in rows:
        for j in range(CAT_LEN):
            catMatch = (a[i] == j)
            sums[j] = mux(catMatch, sums[j] + b[i], sums[j])

    # Print the sums for each category
    @for_range_opt(CAT_LEN)
    def _(i):
        print_ln("Category %s: %s", i, sums[i].reveal())

if __name__ == "__main__":
    compiler.compile_func()