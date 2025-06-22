from Compiler.library import print_ln, for_range_opt
from Compiler.compilerLib import Compiler
from Compiler.types import sint, cint, Array
from Compiler.GC.types import sbitintvec
from Compiler.oram import OptimalORAM


CAT_LEN = 4

usage = "usage: %prog [options] [args]"
compiler = Compiler(usage=usage)

# Options for defining the input matrices and their dimensions
compiler.parser.add_option("--rows", dest="rows", type=int, help="Number of rows for the input matrices)")

compiler.parser.add_option("--aggregation", dest="aggregation", type=str, help="Type of aggregation to be performed (sum, average, freq(uencies), st(d)ev)")
compiler.parser.add_option("--group_by", dest="group_by", type=str, help="Columns to group by (2 max) (e.g ab for Alice's first column and Bob's first column")
compiler.parser.add_option("--value_col", dest="value_col", type=str, help="Value column (not needed for mode and freq.) (e.g b for Bob's column)")

compiler.parse_args()
if not compiler.options.rows:
    compiler.parser.error("--rows required")

#function_name = f"xtabs-{compiler.options.aggregation}-{len(compiler.options.group_by)}"    # e.g. xtabs-sum-2

def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)


def get_arrays(rows, group_by_col, value_col, secret_type):
    group_by_party = 0 if group_by_col == 'a' else 1
    value_party = 0 if value_col == 'a' else 1
    
    group_by = Array(rows, sint)
    value = Array(rows, secret_type)

    group_by.input_from(group_by_party)
    value.input_from(value_party)

    return group_by, value


def get_arrays_bin(rows, group_by_col, value_col, secret_type, oram):
    group_by_party = 0 if group_by_col == 'a' else 1
    value_party = 0 if value_col == 'a' else 1
    
    group_by = OptimalORAM(rows, sbitintvec) if oram else Array(rows, sbitintvec)
    value = Array(rows, secret_type)

    group_by.input_from(group_by_party)
    value.input_from(value_party)

    return group_by, value


def xtabs_sum1(secret_type, cat_len=CAT_LEN):
    max_rows = compiler.options.rows
    group_by, values = get_arrays(max_rows, 
                                  compiler.options.group_by, 
                                  compiler.options.value_col,
                                  secret_type)

    sums = Array(cat_len, secret_type)
    categories = Array(cat_len, cint)

    for i in range(cat_len):
        sums[i] = secret_type(0)
        categories[i] = cint(i)

    @for_range_opt(max_rows)
    def _(i):
        for j in range(cat_len):
            sums[j] = mux(group_by[i] == categories[j], sums[j] + values[i], sums[j])

    
    for i in range(cat_len):
        print_ln("Sum %s: %s", i, sums[i].reveal())


def xtabs_sum2(secret_type, cat_len=CAT_LEN):
    max_rows = compiler.options.rows
    group_by, values = get_arrays(max_rows, 
                                  compiler.options.group_by, 
                                  compiler.options.value_col,
                                  secret_type)

    sums = Array(cat_len, secret_type)
    categories = Array(cat_len, cint)

    for i in range(cat_len):
        sums[i] = secret_type(0)
        categories[i] = cint(i)

    @for_range_opt(max_rows)
    def _(i):
        for j in range(cat_len):
            sums[j] = mux(group_by[i] == categories[j], sums[j] + values[i], sums[j])

    
    for i in range(cat_len):
        print_ln("Sum %s: %s", i, sums[i].reveal())


#@compiler.register_function(function_name)
@compiler.register_function("xtabs")
def main():
    oram = 'oram' in compiler.prog.args # The binary circuit ORAM code uses an optimization that reduces the cost of bit-vector AND in the context of dishonest-majority semi-honest computation
    secret_type = None

    if compiler.prog.options.binary != 0: # If program is being compiled for binary circuits
        print("----------------------------------------------------------------")
        print("Compiling for binary circuits")
        print("----------------------------------------------------------------")
        secret_type = sbitintvec.get_type(int(compiler.prog.options.binary))

    else:
        secret_type = sint
        xtabs_sum1(secret_type)


if __name__ == "__main__":
    compiler.compile_func()