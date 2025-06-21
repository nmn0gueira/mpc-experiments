from Compiler.library import print_ln, for_range_opt
from Compiler.compilerLib import Compiler
from Compiler.types import sint, sfix, Matrix, Array
from Compiler.GC.types import sbits, sbitintvec, sbitvec
from Compiler.oram import OptimalORAM

CAT_LEN = 4

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)

# Options for defining the input matrices and their dimensions
compiler.parser.add_option("--rows", dest="rows", type=int, help="Number of rows for the input matrices)")

compiler.parser.add_option("--aggregation", dest="aggregation", type=str, help="Type of aggregation to be performed (sum, average, freq(uencies), st(d)ev)")
compiler.parser.add_option("--group_by", dest="group_by", type=str, help="Columns to group by (2 max) (e.g ab for Alice's first column and Bob's first column")
compiler.parser.add_option("--value_col", dest="label", type=str, help="Value column (not needed for mode and freq.) (e.g b for Bob's column)")

compiler.parse_args()
if not compiler.options.rows:
    compiler.parser.error("--rows required")

#function_name = f"xtabs-{compiler.options.aggregation}-{len(compiler.options.group_by)}"    # e.g. xtabs-sum-2

def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)


def xtabs_sum(secret_type, oram, binary):
    max_rows = compiler.options.rows

    alice = Array(max_rows, secret_type)
    bob = Array(max_rows, secret_type)

    alice.input_from(0, binary=binary)
    bob.input_from(1, binary=binary)

    if oram:
        sums = OptimalORAM(CAT_LEN, secret_type)
        
        for i in range(CAT_LEN):
            sums[i] = secret_type(0)
        
        @for_range_opt(max_rows)
        def _(i):
            sums[alice[i]] += bob[i]

    else:
        sums = Array(CAT_LEN, secret_type)
        categories = Array(CAT_LEN, secret_type)

        for i in range(CAT_LEN):
            sums[i] = secret_type(0)
            categories[i] = secret_type(i)

        @for_range_opt(max_rows)
        def _(i):
            for j in range(CAT_LEN):
                sums[j] = mux(alice[i] == categories[j], sums[j] + bob[i], sums[j])

    
    for i in range(CAT_LEN):
        print_ln("Sum %s: %s", i, sums[i].reveal())


#@compiler.register_function(function_name)
@compiler.register_function("xtabs")
def main():
    oram = 'oram' in compiler.prog.args # The binary circuit ORAM code uses an optimization that reduces the cost of bit-vector AND in the context of dishonest-majority semi-honest computation
    binary = 'binary' in compiler.prog.args
    secret_type = None

    if compiler.prog.options.binary != 0: # If program is being compiled for binary circuits
        print("----------------------------------------------------------------")
        print("Compiling for binary circuits")
        print("----------------------------------------------------------------")
        secret_type = sbitintvec.get_type(int(compiler.prog.options.binary))

    else:
        secret_type = sint
    
    xtabs_sum(secret_type, oram, binary)


if __name__ == "__main__":
    compiler.compile_func()