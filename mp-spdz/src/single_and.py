from Compiler.GC.types import sbit
from Compiler.library import print_ln
from Compiler.compilerLib import Compiler

compiler = Compiler()

@compiler.register_function('single_and')
def main():
    a =  sbit.get_input_from(0)
    b =  sbit.get_input_from(1)

    print_ln("Alice's and Bob agree: %s", a.bit_and(b).reveal())

if __name__ == "__main__":
    compiler.compile_func()