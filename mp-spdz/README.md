# MP-SPDZ
MP-SPDZ implements a large number of secure multi-party computation (MPC) protocols. It was developed for benchmarking various MPC protocols in a variety of security models such as honest and dishonest majority, semi-honest/passive and malicious/active corruption. The underlying technologies span secret sharing, homomorphic encryption, and garbled circuits. 

The source-code is available on [Github](https://github.com/data61/MP-SPDZ) and it also provides [additional documentation online](https://mp-spdz.readthedocs.io/en/latest/).

To test the sample programs, you will need to either: 
1. Install MP-SPDZ using a binary or source distribution and run the programs locally (potentially using a dev container) - better for rapid testing.
2. Use the root Dockerfile to run the programs in a container - better for testing in a distributed environment and for production while keeping the environment clean.

## Binary or Source Distribution
To setup the environment, you can either use the dev container with the provided Dockerfile or install the required dependencies (listed in the Dockefile) manually.

### Installation
After setting up your environment, you can run the installation script to install MP-SPDZ:
```bash	
./install.sh <fromsource>
```
where `<fromsource>` is either `yes` or `no`. Note that building from source will take longer.

### Usage
To simplify the usage of MP-SPDZ locally while keep the environment uncluttered, a few are provided to scripts to help with the data setup, compilation, and running of the programs. The scripts are located in the `scripts` folder.

#### Setup Data
To run the programs, you will need to setup sample data. You can do this by running the `setup_data.sh` script:
```bash
scripts/setup_data.sh -g -c <program>
```
where `-g` and `-c` are optional flags to generate the data and copy it to `MP-SPDZ/Player-Data`, respectively, and `<program>` is the name of the program you want to setup the data for.

#### Compiling
To run a program, you will first need to compile it. You can do this by running the `compile.sh` script:
```bash	
scripts/compile.sh <program> <protocol_options> <program_args>
```
where `<program>` is the name of the program you want to compile that is in the `src` folder. The `<protocol_options>` arguments can include more than one as per the MP-SPDZ documentation. The `<program_args>` are the arguments that you want to pass to the program, which can also include more than one.

> This script is just to facilitate the compilation process. It copies the program to the MP-SPDZ directory and runs the MP-SPDZ compilation process.

#### Running
To run a program, you can use the `run.sh` script:
```bash
scripts/run.sh <protocol_script> <program> <protocol_script_args>
```
where `<protocol_script>` is the bash script of the protocol that you want to run (with the `.sh` included), `<program>` is the name of the program previously compiled. The `<protocol_script_args>` arguments can include more than one as per the MP-SPDZ documentation.

> Keep in mind that this script is for running in localhost only. To run the compiled program in a distributed environment, you will need to go to the MP-SPDZ directory on each machine and run the compiled program with the preferred protocol binary (.e.g. `./mascot-party.x`).


## Running in Docker

The `Dockerfile` provided is a modified version of the one provided in the MP-SPDZ repository. You can see the `Dockerfile` for more examples of how it can be used (remaining from the original MP-SPDZ repository).

### Localhost
Build the Docker image using the provided Dockerfile (with `mascot` as an example):
```bash
docker build --tag mpspdz:mascot-party --build-arg machine=mascot-party.x --build-arg src=linreg .
```
Then, run the Docker container:
```bash
docker run --rm -it mpspdz:mascot-party ./Scripts/mascot.sh linreg
```

### Distributed
--TBD--


## TBD
### Potential Optimizations
- Certain operations can be optimized with multiple threads like for_range_opt_multithread() and certain protocol parameters
- Certain protocols can run the required preprocessing without running the actual computation. This means we can separate them into an offline and online phase manually. The current supported protocols are `mascot`, `cowgear`, `mal-shamir`, `semi`, `semi2k`, and `hemi`. Check MP-SPDZ docs for more info,
- Check cheat sheet for other optimizations.

#### Cross-tabulation (Sum)
- Decide if it is better to either: 
	1. Use the first input of one party as the number of categories and the first input of another to be the input size and only compile once. 
		1. Pros: Only compiles once
		2. Cons: Still requires specifying a max size when compiling.
        3. Notes: If choosing this one have the groupby column and values column provider given in public input if possible.
	2. Have both input size and number of categories as specified compiler args and compile each time it is needed
		1. Pros: Input size is known at compile time
		2. Cons: Compiling more than once
        3. Notes: If choosing this one have the groupby column and values column provider given in the compiler args.
