#!/bin/bash

set -e

program=$1

MILLIONAIRE_ALICE=70
MILLIONAIRE_BOB=20

# Output from Bob is supressed to not clutter the terminal
case $program in
    "millionaire" )
        ./build/bin/millionaire 1 7777 $MILLIONAIRE_ALICE & ./build/bin/millionaire 2 7777 127.0.0.1 $MILLIONAIRE_BOB > /dev/null
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
        ./build/bin/xtabs 1 7777 $aggregation data/xtabs/2.1.dat & ./build/bin/xtabs 2 7777 127.0.0.1 $aggregation data/xtabs/32.2.dat > /dev/null
        # if num_cols is 2, use 2 aggregation cols and 1 value col
        # etc
        ;;
    "linreg" )
        ./build/bin/linreg 1 7777 data/linreg/32.1.dat & ./build/bin/linreg 2 7777 127.0.0.1 data/linreg/32.2.dat > /dev/null
        ;;
    "hist2d" )
        ./build/bin/hist2d 1 7777 data/hist2d/32.1.dat & ./build/bin/hist2d 2 7777 127.0.0.1 data/hist2d/32.2.dat > /dev/null
        ;;
    * )
        echo "Unknown program: $program"
        exit 1
    esac