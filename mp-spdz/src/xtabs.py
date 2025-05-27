from Compiler.library import print_ln, for_range_opt
from Compiler.compilerLib import Compiler
from Compiler.types import sint, sfix, Matrix, Array

INPUT_SIZE = 10000
CAT_LEN = 4

compiler = Compiler()

def mux(cond, trueVal, falseVal):
    return cond.if_else(trueVal, falseVal)

# You can run this program in interactive mode to pass the inputs directly
@compiler.register_function('xtabs')
def main():
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
	


if __name__ == "__main__":
    compiler.compile_func()