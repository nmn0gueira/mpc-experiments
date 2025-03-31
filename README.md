# EMP-toolkit Experiments

This repository contains examples of functions implemented using the EMP-toolkit library for secure two-party computation. The functions serve
to benchmark the performance of the library and to provide a reference for future implementations.

## Development
Development was done through a development container using the repositories' `devcontainer.json` file along with its Dockerfile. This will allow you to develop in a consistent environment with all the necessary dependencies.

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
 - `millionaire`: Determine which of two numbers is larger. Basic control test program.
 - `xtabs`: Computation of cross-tabulation/pivot table. Groups by a column or more and computes an aggregation on the values of another column. Current aggregations include: 
    - sum (grouping by a single column)
    - average (grouping by a single column)
    - frequency/mode
- `linreg`: Linear regression. Computes the coefficients of a linear regression model. By default, uses the standard scaler to scale the data.
- `hist2d`: 2D histogram, also known as a density heatmap. Computed by grouping a set of points in a 2D space into bins and counting the number of points in each bin. The bins are defined by the user and the function will return the counts for each bin (**this last part is TBD**).


## Potential Improvements
### Functions
- `filter`: A universal filtering function that allows to filter a dataset based on a set of conditions.
- `nan_handling`: A function to handle NaN values in a dataset.

### Extensions
- `hist2d`: 
    - Extend it to `histnd`, an N-dimensional histogram. 
    - Implement other aggregations for it, such as the mean.

### Optimizations
- `xtabs`: 
    - The `average` function can be optimized by revealing the sum and count, allowing not only working with only integers for the circuit but also not needing to do the division inside the circuit.
- `linreg`:
    - As of now, I do not know yet if the current implementation is as optimal as can be in a boolean circuit.
- `hist2d`:
    - Change the usage of floats to integers trading a potential slight loss of precision in the binning for a more efficient computation.


## Additional Notes
The programs in this repository all assume that the input for both parties is already matched (as if each record in order from both parties had the same ID) since the focus is on the computation itself and not the matching.
