# MP-SPDZ
MP-SPDZ implements a large number of secure multi-party computation (MPC) protocols. It was developed for benchmarking various MPC protocols in a variety of security models such as honest and dishonest majority, semi-honest/passive and malicious/active corruption. The underlying technologies span secret sharing, homomorphic encryption, and garbled circuits. 

The source-code is available on [Github](https://github.com/data61/MP-SPDZ) and it also provides [additional documentation online](https://mp-spdz.readthedocs.io/en/latest/).

To test the sample programs, you will need to either: 
1. Install MP-SPDZ using a binary or source distribution and run the programs locally (potentially using a dev container).
2. Use the root Dockerfile to run the programs in a container,

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
To run the programs, you will need to prepare program input. You can do this by running the `iprep.sh` script:
```bash
scripts/iprep.sh [options] <program_name> [<geninput_args>]
```
where `<program_name>` is the name of the program compiled (or to be compiled) that is in the `src` folder. For more information on the available options, you can run `scripts/iprep.sh --help`.


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

To run the examples in a Docker environment, you have two options: build a custom Docker image using the provided `Dockerfile`, or use the included `compose.yml` file.

The supplied Dockerfile is a modified version of the one from the MP-SPDZ repository. Refer to it for additional usage examples preserved from the original source.

### Quick Start
Build the image for a specific computation machine (e.g., mascot-party.x) and program (e.g., linreg):
```bash
docker build \ 
--tag mpspdz:mascot-linreg \
--build-arg machine=mascot-party.x \
--build-arg src=linreg . \
.
```
> Note: You can also use the included `docker-build.sh` to simplify Docker builds. Run `scripts/docker-build.sh --help` for options.

Then, run the container:
```bash
docker run --rm -it mpspdz:mascot-linreg ./Scripts/mascot.sh linreg
```

Alternatively, you can use Docker Compose to build and run with a similar setup. First, make sure the `compose.yaml` is configured appropriately (by exporting the necessary variables). Then:
```bash
docker compose up
```

### Optimizing Build Time
The Dockerfile defines multiple build stages, such as `buildenv`, `machine`-specific stages, and `program`. You can pre-build intermediate targets to speed up future builds:
```bash
docker build --target buildenv -t mpspdz:buildenv .
docker build --target machine -t mpspdz:mascot-party --build-arg machine=mascot-party.x .
```

Once these are cached, rebuilding the final stage (i.e. compiling a different program) is much faster:
```bash
docker build \
  --target program \
  --tag mpspdz:mascot-party \
  --build-arg machine=mascot-party.x \
  --build-arg src=other_program \
  .
```

This is especially useful when experimenting with different programs while reusing the same machine configuration.

## TBD
### Optimizations
- `hist2d`: 
	- The hist2d function is very slow in arithmetic protocols which makes sense as there are much more logic operations than arithmetic ones. Upgrading hardware might help, but it might be a good idea to try to implement a version that uses binary protocols instead.
	- See if there is a way to access the max and min values for sint, sfix and sfloat types in the code (e.g. `sint.max_value()`). Additionally, current hard-coded values for sint and sfloat are wrong (copied from sfix).
- `linreg`: 
	- Potentially add a version of simple_linreg that separates into train and test phases. Would make use of the input being stored in columns as in sgd linreg.
	- Add a version of sgd_linreg that uses user defined regular torch neural network as this might allow extensions such as poly feats or lasso/ridge regression (check linreg code comments for more info).