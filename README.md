# EMP-toolkit Experiments

This repository contains examples of functions implemented using the EMP-toolkit library for secure two-party computation. The functions serve
to benchmark the performance of the library and to provide a reference for future implementations.

## Development
Development was done through a development container using the repositories' `devcontainer.json` file. This will allow you to develop in a consistent environment with all the necessary dependencies.

If you want to use the development container, you will need to have Docker installed on your machine and a code editor that supports devcontainers (e.g. Visual Studio Code).

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
Some examples may have slighly different arguments where it is best to check the usage by simplying executing the program without arguments. In general, to run an example you can run the following command for Alice:

```bash
./bin/<ex> 1 <port> <input file>
```

And the following for Bob:

```bash
./bin/<ex> 2 <port> <ip> <input file>
```


## Example functions

Current functions include:
 - 'millionaire': Determine which of two numbers is larger. Basic control test program.
 - 'xtabs': Computation of cross-tabulation/pivot table. Groups by a column or more and computes an aggregation on the values of another column. Current aggregations include: 
    - sum (grouping by a single column)
    - average (grouping by a single column)
    - frequency/mode
- 'linreg': Linear regression. Computes the coefficients of a linear regression model.

### TBD (for now)
- 'standard_scaler': Add the standard scaler to the linear regression model as the default option.
- 'hist_2d': 2D histogram. Computes the frequency of each pair of values in two columns.
