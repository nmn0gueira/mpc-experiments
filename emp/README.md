# EMP-toolkit
EMP-toolkit is a C++ library for secure two-party computation (2PC) based on the Yao's garbled circuits protocol.

## Development
Development was done through a development container using the repositories' `devcontainer.json` file along with its Dockerfile. This will allow you to develop in a consistent environment with all the necessary dependencies.

If you want to use the development container, you will need to have Docker installed on your machine and a code editor that supports devcontainers (e.g., Visual Studio Code).

Otherwise, you will need Ubuntu 24.04 and the required dependencies on your machine:
- software-properties-common
- cmake
- build-essential
- libssl-dev
- emp-toolkit (emp-tool, emp-ot, emp-sh2pc, emp-ag2pc)


## Compilation

```bash
mkdir build
cd build
cmake ..
make
```

## Usage
### Generating input data
To run the examples, you will need sample data. This data can be generated with the python script included in the repository. Usage of the script is as follows:

```bash
python3 geninput.py -e <ex> -n <bitsize> -l <numrows>
```

The default for -n and -l is 32 and 10 respectively. 


### Running the examples
Some examples may have slightly different arguments where it is best to check the usage by simplifying executing the program without arguments. In general, to run an example you can run the following command for Alice:

```bash
./bin/<ex> 1 <port> <input file>
```

And the following for Bob:

```bash
./bin/<ex> 2 <port> <ip> <input file>
```


## TBD
This sections includes work that has not been completed yet for this particular framework.

### Functionality
- `xtabs`: 
    - Add support for grouping by multiple columns.
- `linreg`: 
    - Add support for more than one feature.

### Optimizations
- `xtabs`: 
    - The `average` function can be optimized by revealing the sum and count, allowing not only working with only integers for the circuit but also not needing to do the division inside the circuit.
- `linreg`:
    - As of now, I do not know yet if the current implementation is as optimal as can be in a boolean circuit.
- `hist2d`:
    - Change the usage of floats to integers trading a potential slight loss of precision in the binning for a more efficient computation.
