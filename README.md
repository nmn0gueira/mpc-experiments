# MPC Experiments
This repository contains example implementations of functions using various general-purpose multi-party computation (MPC) frameworks for secure two-party computation. The primary goal is to benchmark the performance of different frameworks and potentially serve as a reference for future implementations.

## Development
Each framework is located in its own directory, containing the implementation of the functions and a README file with specific instructions to set up its build environment. The README files also include instructions on how to run the test programs and any additional notes specific to that framework.

## Test Programs
The test programs focus on statistical analysis and are designed to be simple yet representative of common data processing tasks. Each function is implemented in multiple frameworks to compare performance and usability.

Below is a general overview of the implemented functions. For detailed information on each framework’s implementation, refer to the README files inside their respective directories.

 - `millionaire`: Determines which of two numbers is larger. A basic control program to validate setup.
 - `xtabs`: Cross-tabulation (pivot table). Groups by one or two columns and performs aggregations on another column. Supported aggregations: 
    - sum
    - average
    - frequency count
    - mode (does not reveal frequency counts)
- `linreg`: Linear regression. Computes regression coefficients after applying standard scaling to the (numerical) input data.
- `hist2d`: 2D histogram (density heatmap). Bins 2D points into a grid and counts the number of points per bin.


## Potential Additions
### Data Preprocessing
- `filter`: A universal filtering function that allows to filter a dataset based on a set of conditions.
- `nan_handling`: A function to handle NaN values in a dataset.

### Program Extensions
- `xtabs`: 
  - Add support for more aggregations, such as the median or the standard deviation.
- `linreg`: 
  - Extend it to support multiple target variables (multivariate regression).
  - Extend it to support polynomial regression.
  - Add support for categorical variables (e.g., one-hot encoding).
  - Implement regularization techniques like Lasso or Ridge regression.
- `hist2d`: 
  - Extend it to `histnd`, an N-dimensional histogram. 
  - Implement other aggregations for it, such as the mean.
  - Allow users to define number of bins and, potentially, the range of the bins.

### Frameworks
Support for other frameworks may be added in the future. In particular, the following frameworks are of interest (at the moment):
- EzPC
- ABY 



## Additional Notes
All programs assume pre-aligned inputs—each record from one party corresponds to the same index as the record from the other party. The focus is solely on computation, not record linkage.

### Framework-Specific Notes
Framework-specific implementations that do not fully include the functionality mentioned in this README file are marked in the `TBD` section of the README file corresponding to that framework. The `TBD` section also includes any additional work that has not been completed yet for this particular framework.

## About
This software was developed as part of my thesis: **Privacy-Preserving Analysis of Misinformation Data**

The repository and its structure were inspired by the paper *SoK: General Purpose Compilers for Secure Multi-Party Computation* and its companion [repository](https://github.com/MPC-SoK/frameworks/), which provided an excellent starting point for working with MPC frameworks.