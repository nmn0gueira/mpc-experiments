# MP-SPDZ
MP-SPDZ implements a large number of secure multi-party computation (MPC) protocols. It was developed for benchmarking various MPC protocols in a variety of security models such as honest and dishonest majority, semi-honest/passive and malicious/active corruption. The underlying technologies span secret sharing, homomorphic encryption, and garbled circuits. 

The source-code is available on [Github](https://github.com/data61/MP-SPDZ) and it also provides [additional documentation online](https://mp-spdz.readthedocs.io/en/latest/).

## Installation
To setup the environment, you can either use the dev container with the provided Dockerfile or install the required dependencies (listed in the Dockefile) manually.

After setting up your environment, you can run the installation script to install MP-SPDZ:
```bash	
./install.sh <fromsource>
```
where `<fromsource>` is either `yes` or `no`.


## Usage
### Setup Data
To run the programs, you will need to setup sample data. You can do this by running the `setup_data.sh` script:
```bash
./setup_data.sh -g -c <program>
```
where `-g` and `-c` are optional flags to generate the data and copy it to `MP-SPDZ/Player-Data`, respectively, and `<program>` is the name of the program you want to setup the data for.

### Compiling
To run a program, you will first need to compile it. You can do this by running the `compile.sh` script:
```bash	
./compile.sh <program> <protocol_options> <program_args>
```
where `<program>` is the name of the program you want to compile that is in the `src` folder. The `<protocol_options>` arguments can include more than one as per the MP-SPDZ documentation. The `<program_args>` are the arguments that you want to pass to the program, which can also include more than one.

> This script is just to facilitate the compilation process. It copies the program to the MP-SPDZ directory and runs the MP-SPDZ compilation process.

### Running
To run a program, you can use the `run.sh` script:
```bash
./run.sh <protocol_script> <program> <protocol_script_args>
```
where `<protocol_script>` is the bash script of the protocol that you want to run (with the `.sh` included), `<program>` is the name of the program previously compiled. The `<protocol_script_args>` arguments can include more than one as per the MP-SPDZ documentation.

> Keep in mind that this script is for running in localhost only. To run the compiled program in a distributed environment, you will need to go to the MP-SPDZ directory on each machine and run the compiled program with the preferred protocol binary (.e.g. `./mascot-party.x`).

## TBD
### Fixes
- Fix installing from source (might need it if I need to change some parameters for certain protocols)

### Potential Optimizations
- Certain operations can be optimized with multiple threads like for_range_opt_multithread() and certain protocol parameters
- Certain protocols can run the required preprocessing without running the actual computation. This means we can separate them into an offline and online phase manually.