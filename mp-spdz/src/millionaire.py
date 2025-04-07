from Compiler.types import sint
from Compiler.library import print_ln
from Compiler.compilerLib import Compiler

compiler = Compiler()

@compiler.register_function('millionaire')
def main():
    a =  sint.get_input_from(0)
    b =  sint.get_input_from(1)

    print_ln("Alice's richer than Bob: %s", (a > b).reveal())

if __name__ == "__main__":
    compiler.compile_func()