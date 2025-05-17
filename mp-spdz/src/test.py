from Compiler.types import sfix
from Compiler.library import print_ln
from Compiler.compilerLib import Compiler

compiler = Compiler()

@compiler.register_function('test')
def main():

    a = sfix.Tensor([1000, 2])
    b = sfix.Tensor([1000, 2])

    a.input_from(0, binary=True)
    b.input_from(1, binary=True)

    print_ln("Tensor a: %s", a.reveal())
    print_ln("Tensor b: %s", b.reveal())

if __name__ == "__main__":
    compiler.compile_func()