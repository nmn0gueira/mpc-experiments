# MP-SPDZ
MP-SPDZ implements a large number of secure multi-party computation (MPC) protocols. It was developed for benchmarking various MPC protocols in a variety of security models such as honest and dishonest majority, semi-honest/passive and malicious/active corruption. The underlying technologies span secret sharing, homomorphic encryption, and garbled circuits. 

The source-code is available on [Github](https://github.com/data61/MP-SPDZ) and it also provides [additional documentation online](https://mp-spdz.readthedocs.io/en/latest/).

To test the sample programs, you will need to either: 
1. Install MP-SPDZ using a binary or source distribution and run the programs locally (potentially using a dev container).
2. Use the root Dockerfile to run the programs in a container,

Regardless, you will need to install the required python dependencies. The usage of a virtual environment is recommended.
1. (Optional) Create a virtual environment and activate it:
```bash
python3 -m venv venv
source venv/bin/activate
```
2. Update pip and install the required dependencies:
```bash
pip install --upgrade pip
pip install -r requirements.txt
```

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
scripts/iprep.sh -g -c <program> [<geninput_args>]
```
where `-g` and `-c` are optional flags to generate the data and copy it to `MP-SPDZ/Player-Data`, respectively, and `<program>` is the name of the program you want to setup the data for. The `<geninput_args>` are the arguments that you want to pass to the `geninput.py` script.


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
--target program \
--tag mpspdz:mascot-party \
--build-arg machine=mascot-party.x \
--build-arg src=linreg . \
.
```
Then, run the container:
```bash
docker run --rm -it mpspdz:mascot-party ./Scripts/mascot.sh linreg
```

Alternatively, use Docker Compose to build and run with a similar setup. First, make sure the `compose.yaml` is configured appropriately (by exporting the necessary variables). Then:
```bash
docker compose up
```

### Optimizing Build Time
The Dockerfile defines multiple build stages, such as `buildenv`, `machine`-specific stages, and `program`. You can pre-build intermediate targets to speed up future builds:
```bash
docker build --target buildenv -t mpspdz:buildenv .
docker build --target mascot-party -t mpspdz:mascot-party --build-arg machine=mascot-party.x .
```
Once these are cached, rebuilding the final stage (e.g., compiling a different program) is much faster:
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
### Potential Optimizations
- Certain operations can be optimized with multiple threads like for_range_opt_multithread() and certain protocol parameters
- Certain protocols can run the required preprocessing without running the actual computation. This means we can separate them into an offline and online phase manually. The current supported protocols are `mascot`, `cowgear`, `mal-shamir`, `semi`, `semi2k`, and `hemi`. Check MP-SPDZ docs for more info,
- Check cheat sheet for other optimizations.
- Experiment with homomoprhic encryption with use_ntl=1 (especially galois field protocols).
- Semi honest protocols can use the special probabilistic truncation function (used for linreg, for example). Leads to faster multiplications.
- The --direct command line argument for fewer party protocols.
- Configure multithreading as a compiler argument both for generating input and regular programs.

#### Cross-Tabulation
If possible, have it as one file that can be compiled into different named functions to avoid both performance loss and clutter.

#### Compilation process
- Decide if it is better to either: 
	1. Have arguments and input shapes be specified by public_input():
		1. Pros: Only compiles once
		2. Cons: Still requires specifying a max size when compiling. May make the runtime slower.
	2. Have input size and arguments (or only input size) known at compile time:
		1. Pros: Makes runtime faster.
		2. Cons: Compiling more than once

#### Private Inputs
Right now, the choice in how private inputs are handled is to either:
1. The program to be ran itself has the input_tensor_via() function that is handled differently depending on the computing party. This makes local tests only feasible with docker compose
2. Have an intermediary to generate binary or raw input from the input_tensor_via() function like iprep.py which will read the csv when compiled and generate the relevant player data. This data can then be used as input for other programs with input_from(). This facilitates local testing and the compiling by max size method (probably)