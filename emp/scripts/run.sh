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
LINREG_INPUT_A=$DATA_DIR/linreg/alice
LINREG_INPUT_B=$DATA_DIR/linreg/bob
HIST2D_INPUT_A=$DATA_DIR/hist2d/alice
HIST2D_INPUT_B=$DATA_DIR/hist2d/bob

# Additonal parameters
XTABS_NUM_CATEGORIES_1=4
XTABS_NUM_CATEGORIES_2=4
XTABS_GROUPBY_ONE=a0
XTABS_GROUPBY_TWO=a0b0
XTABS_VALUE_COLUMN=b1

HIST2D_NUM_EDGES_X=6
HIST2D_NUM_EDGES_Y=6

# Default values
alice=false
bob=false
address=127.0.0.1

usage() {
    echo "Usage: $0 [-a] [-b <alice_address>] <program_name> <input_size> [<args>]"
    echo ""
    echo "Options:"
    echo "  -a                Run as Alice"
    echo "  -b <address>      Run as Bob (connect to Alice at <address>)"
    echo "  -a -b             Run both Alice and Bob (default if no options are given)"
    echo ""
    echo "Programs:"
    echo "  millionaire                                                             Secure comparison of two numbers"
    echo "  xtabs <aggregation> <groupby> [<num_categories_1>] [<num_categories_2>] Cross-tabulation [(s)um | (a)vg | | a(v)g_fast | (m)ode | (f)req | st(d)ev ; number of group by columns (1 or 2)]"
    echo "  linreg                                                                  Linear regression"
    echo "  hist2d <mode> [<num_edges_x>] [<num_edges_y>]                           2D histogram [(i)nteger or (f)loat usage for binning]"
}

build_command() {
    local first_command=$1
    local second_command=$2
    
    if { [ "$alice" = true ] && [ "$bob" = true ]; } || { [ "$alice" = false ] && [ "$bob" = false ]; } then
        echo "Running both Alice and Bob"
        $first_command & $second_command > /dev/null 
        wait   
    elif [ "$alice" = true ]; then
        echo "Running Alice"
        $first_command
    elif [ "$bob" = true ]; then
        echo "Running Bob"
        $second_command
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
input_size=$2

if [ -z "$program" ] || [ -z "$input_size" ]; then
    echo "Program name and input size are required."
    usage
    exit 1
fi

shift 2

case $program in
    "millionaire" )
        alice_command="./build/bin/millionaire $PARTY_A $PORT $MILLIONAIRE_INPUT_A"
        bob_command="./build/bin/millionaire $PARTY_B $PORT $address $MILLIONAIRE_INPUT_B"
        ;;
    "xtabs" ) 
        aggregation=$1
        groupby=$2
        num_categories_1=${3:-$XTABS_NUM_CATEGORIES_1}
        num_categories_2=${4:-$XTABS_NUM_CATEGORIES_2}
        if [ -z "$aggregation" ] || [ -z "$groupby" ]; then
            echo "xtabs requires an aggregation type ((s)um | (a)vg | | a(v)g_fast | (m)ode | (f)req | st(d)ev) and number of group by columns (1 or 2)"
            exit 1
        fi
        if [ "$groupby" = "1" ]; then
            groupby=$XTABS_GROUPBY_ONE
        elif [ "$groupby" = "2" ]; then
            groupby=$XTABS_GROUPBY_TWO
        else
            echo "Invalid group by column count. Use 1 or 2."
            exit 1
        fi
        alice_command="./build/bin/xtabs $PARTY_A $PORT $input_size $aggregation $num_categories_1 $num_categories_2 $groupby $XTABS_VALUE_COLUMN $XTABS_INPUT_A"
        bob_command="./build/bin/xtabs $PARTY_B $PORT $address $input_size $aggregation $num_categories_1 $num_categories_2 $groupby $XTABS_VALUE_COLUMN $XTABS_INPUT_B"
        ;;
    "linreg" )
        alice_command="./build/bin/linreg $PARTY_A $PORT $input_size $LINREG_INPUT_A"
        bob_command="./build/bin/linreg $PARTY_B $PORT $address $input_size $LINREG_INPUT_B"
        ;;
    "hist2d" )
        mode=$1
        num_edges_x=${2:-$HIST2D_NUM_EDGES_X}
        num_edges_y=${3:-$HIST2D_NUM_EDGES_Y}
        if [ -z "$mode" ]; then
            echo "hist2d requires choosing a mode for how the type used for the defined bins ((i)nt or (f)loat)"
            exit 1
        fi
        alice_command="./build/bin/hist2d $PARTY_A $PORT $input_size $mode $num_edges_x $num_edges_y $HIST2D_INPUT_A"
        bob_command="./build/bin/hist2d $PARTY_B $PORT $address $input_size $mode $num_edges_x $num_edges_y $HIST2D_INPUT_B"
        ;;
    * )
        echo "Unknown program: $program"
        usage
        exit 1
    esac

build_command "$alice_command" "$bob_command"