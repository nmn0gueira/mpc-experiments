#!/bin/bash
# input preparation script
set -e

split=false
binary=false
columns=""
by_column=false

verbose=false

MP_SPDZ_PATH="MP-SPDZ"
destination_dir="$MP_SPDZ_PATH/Player-Data"
generated_data_dir="data/"

usage() {
    echo "Usage: $0 [options] <program_name> [<geninput_args>]"
    echo ""
    echo "Options:"
    echo "  -s, --split         Split the input data into multiple files (e.g., train data and test data) (default: false)"
    echo "  -b, --binary        Use binary format for the input data (default: false)"
    echo "  -c <columns>        Specify columns to copy (e.g., a0b1 for alice's column 0 and bob's column 1)"
    echo "  --by_column         Store input by column instead of by row (used for programs that read input by column instead of by row such as 'linreg' and 'xtabs')"
    echo "  -v, --verbose       Enable verbose output (compile output will be shown)"
    echo "  -h, --help          Show this help message"
    echo ""
    echo "Arguments:"
    echo "  <program_name>      Name of the program to run (e.g., linreg, xtabs)"
    echo "  [<geninput_args>]   Additional arguments from geninput.py (e.g., -l 100 for 100 rows)"
    echo ""
    echo "Example:"
    echo "  $0 -s -c a0b0 --by_column linreg -l 10000"
    echo ""
    exit 1
}

csv2spdz() {
    split_option=""
    if $split; then
        split_option="split"
    fi
    binary_option=""
    if $binary; then
        binary_option="binary"
    fi
    columns_option=""
    if [[ -n "$columns" ]]; then
        columns_option="--columns $columns"
    fi
    by_column_option=""
    if $by_column; then
        by_column_option="by_column"
    fi

    if $verbose; then
        python csv2spdz.py party0 party1 $split_option $binary_option $columns_option $by_column_option
    else
        python csv2spdz.py party0 party1 $split_option $binary_option $columns_option $by_column_option > /dev/null
    fi

}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -s|--split)
            split=true
            shift
            ;;
        -b|--binary)
            binary=true
            shift
            ;;
        -c|--columns)
            if [[ -z "$2" ]]; then
                echo "Error: --columns requires an argument."
                usage
            fi
            columns="$2"
            shift 2
            ;;
        --by_column)
            by_column=true
            shift
            ;;
        -v|--verbose)
            verbose=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        -*)
            usage
            ;;
        *)
            break
            ;;
    esac
done

shift $((OPTIND - 1))

program_name=$1

if [ -z "$program_name" ]; then
    usage
fi

shift

# Actual data gen
python scripts/geninput.py -e $program_name $@

# Copy to mp-spdz dir and convert to mp-spdz input
cp -r $generated_data_dir/$program_name/* $destination_dir/
cp scripts/csv2spdz.py $MP_SPDZ_PATH
cd $MP_SPDZ_PATH
csv2spdz