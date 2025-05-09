#!/bin/bash

set -e

program=$1

if [ -z "$program" ]; then
    echo "Usage: $0 <program>"
    echo "Available programs: millionaire, xtabs, linreg, hist2d"
    exit 1
fi

# Settings
PARTY_A=1
PARTY_B=2
PORT=7777
LOCALHOST=127.0.0.1
DATA_DIR=$(pwd)/data

# Inputs
MILLIONAIRE_A=70
MILLIONAIRE_B=20
XTABS_A=$DATA_DIR/xtabs/2.1.dat
XTABS_B=$DATA_DIR/xtabs/32.2.dat
LINREG_A=$DATA_DIR/linreg/32.1.dat
LINREG_B=$DATA_DIR/linreg/32.2.dat
HIST2D_A=$DATA_DIR/hist2d/32.1.dat
HIST2D_B=$DATA_DIR/hist2d/32.2.dat


# Output from Bob is supressed to not clutter the terminal
case $program in
    "millionaire" )
        ./build/bin/millionaire $PARTY_A $PORT $MILLIONAIRE_A & ./build/bin/millionaire $PARTY_B $PORT $LOCALHOST $MILLIONAIRE_B > /dev/null
        ;;
    "xtabs" ) 
        # Not yet set up to specify different files when needed depending on aggregation. May be changed later
        num_cols=$2    # Unused for now
        aggregation=$3 
        if [ -z "$num_cols" ] || [ -z "$aggregation" ]; then
            echo "Usage for xtabs: $0 <num_cols> <aggregation>"
            exit 1
        fi
        # if num_cols is 1, use 1 aggregation col and 1 value col
        ./build/bin/xtabs $PARTY_A $PORT $aggregation $XTABS_A & ./build/bin/xtabs $PARTY_B $PORT $LOCALHOST $aggregation $XTABS_B > /dev/null
        # if num_cols is 2, use 2 aggregation cols and 1 value col
        # etc
        ;;
    "linreg" )
        ./build/bin/linreg $PARTY_A $PORT $LINREG_A & ./build/bin/linreg $PARTY_B $PORT $LOCALHOST $LINREG_B > /dev/null
        ;;
    "hist2d" )
        ./build/bin/hist2d $PARTY_A $PORT $HIST2D_A & ./build/bin/hist2d $PARTY_B $PORT $LOCALHOST $HIST2D_B > /dev/null
        ;;
    * )
        echo "Unknown program: $program"
        exit 1
    esac