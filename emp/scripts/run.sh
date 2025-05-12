#!/bin/bash

set -e

# Global parameters
PARTY_A=1
PARTY_B=2
PORT=7777
DATA_DIR=$(pwd)/data

# Inputs
MILLIONAIRE_INPUT_A=70
MILLIONAIRE_INPUT_B=20
XTABS_INPUT_A=$DATA_DIR/xtabs/alice
XTABS_INPUT_B=$DATA_DIR/xtabs/bob
LINREG_INPUT_A=$DATA_DIR/linreg/alice/32.alice.dat
LINREG_INPUT_B=$DATA_DIR/linreg/bob/32.bob.dat
HIST2D_INPUT_A=$DATA_DIR/hist2d/alice/32.alice.dat
HIST2D_INPUT_B=$DATA_DIR/hist2d/bob/32.bob.dat

# Additonal parameters
XTABS_GROUPBY_COLUMNS=a0b0
XTABS_VALUE_COLUMN=b1

# Default values
alice=false
bob=false
address=127.0.0.1

usage() {
    echo "Usage: $0 [-a] [-b <address>] <program_name> [args]"
    echo ""
    echo "Options:"
    echo "  -a                Run as Alice"
    echo "  -b <address>      Run as Bob (connect to Alice at <address>)"
    echo "  -a -b             Run both Alice and Bob (default if no options are given)"
    echo ""
    echo "Programs:"
    echo "  millionaire                     Secure comparison"
    echo "  xtabs <aggregation>             Cross-tabulation: (s)um | (a)vg | (m)ode | (f)req"
    echo "  linreg                          Linear regression"
    echo "  hist2d                          2D histogram"
}

build_command() {
    local first_command=$1
    local second_command=$2
    
    if [ "$alice" = true ] && [ "$bob" = true ]; then
        echo "Running both Alice and Bob"
        $first_command & $second_command > /dev/null &
        wait
    elif [ "$alice" = true ]; then
        echo "Running Alice"
        $first_command
    elif [ "$bob" = true ]; then
        echo "Running Bob"
        $second_command
    else
        echo "Running both Alice and Bob (default)"
        $first_command & $second_command > /dev/null &
        wait
    fi
}

while getopts "ab:" opt; do
    case $opt in
        a)  alice=true ;;
        b)  bob=true; address=$OPTARG; echo "Address set to $address" ;;
        \?) echo "Invalid flag"; usage; exit 1 ;;
        :)  echo "Option -$OPTARG requires an argument"; usage; exit 1 ;;
    esac
done
shift $((OPTIND -1))

program=$1

if [ -z "$program" ]; then
    echo "No program specified"
    usage
    exit 1
fi

case $program in
    "millionaire" )
        alice_command="./build/bin/millionaire $PARTY_A $PORT $MILLIONAIRE_INPUT_A"
        bob_command="./build/bin/millionaire $PARTY_B $PORT $address $MILLIONAIRE_INPUT_B"
        ;;
    "xtabs" ) 
        aggregation=$2
        if [ -z "$aggregation" ]; then
            echo "xtabs requires an aggregation type: sum | avg | mode | freq"
            usage
            exit 1
        fi
        alice_command="./build/bin/xtabs $PARTY_A $PORT $aggregation $XTABS_GROUPBY_COLUMNS $XTABS_VALUE_COLUMN $XTABS_INPUT_A"
        bob_command="./build/bin/xtabs $PARTY_B $PORT $address $aggregation $XTABS_INPUT_B"
        ;;
    "linreg" )
        alice_command="./build/bin/linreg $PARTY_A $PORT $LINREG_INPUT_A"
        bob_command="./build/bin/linreg $PARTY_B $PORT $address $LINREG_INPUT_B"
        ;;
    "hist2d" )
        alice_command="./build/bin/hist2d $PARTY_A $PORT $HIST2D_INPUT_A"
        bob_command="./build/bin/hist2d $PARTY_B $PORT $address $HIST2D_INPUT_B"
        ;;
    * )
        echo "Unknown program: $program"
        usage
        exit 1
    esac

build_command "$alice_command" "$bob_command"