# MP-SPDZ
MP-SPDZ implements a large number of secure multi-party computation protocols.

## Installation
To setup the environment, you can either use the dev container with the provided Dockerfile or install the required dependencies (listed in the Dockefile) manually.

After setting up your environment, you can run the installation script to install MP-SPDZ:
```bash	
./install.sh <fromsource>
```
where `<fromsource>` is either `yes` or `no`.


## Usage
To run a program, you will first need to compile it. You can do this by running the `compile.sh` script:
```bash	
./compile.sh <program> <protocol_options> <program_args>
```
where `<program>` is the name of the program you want to compile that is in the `src` folder, The `<protocol_options>` arguments can include more than one as per the MP-SPDZ documentation. The `<program_args>` are the arguments that you want to pass to the program, which can also include more than one.

This script is just to facilitate the compilation process. It just copies the program to the MP-SPDZ directory and runs the MP-SPDZ compilation process.

## TBD
### Fixes
- Fix installing from source (might need it if I need to change some parameters for certain protocols)

### QoL Scripts
- Script to run the compiled program (just localhost)
- Script to generate data for sample programs
- Script to copy data to the correct location in MP-SPDZ folder

### Potential Optimizations
- Certain operations can be optimized with multiple threads like for_range_opt_multithread() and certain protocol parameters (I believe)
- Certain protocols can run the required preprocessing without running the actual computation. This means we can separate them into an offline and online phase manually.
- There are also different ways to do non-linear computation that can be decided upon before compilation.