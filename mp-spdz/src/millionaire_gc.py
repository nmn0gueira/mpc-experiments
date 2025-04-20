from Compiler.library import print_ln
from Compiler.compilerLib import Compiler
from Compiler.GC.types import sbitintvec

BITSIZE = 32

compiler = Compiler()

@compiler.register_function('millionaire-gc')
def main():
    siv32 = sbitintvec.get_type(BITSIZE)

    a = siv32.get_input_from(0)
    b = siv32.get_input_from(1)

    print_ln("Alice's richer than Bob: %s", (a > b).reveal())

if __name__ == "__main__":
    compiler.compile_func()