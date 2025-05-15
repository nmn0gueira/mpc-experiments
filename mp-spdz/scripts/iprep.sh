#!/bin/bash
# input preparation script
set -e

generate=false
copy=false
destination_dir="MP-SPDZ/Player-Data"
generated_data_dir="data/"

usage() {
    echo "Usage: $0 [-g] [-c] <program_name> [<geninput_args>]"
    echo "  -g                Generate data"
    echo "  -c                Copy generated data"
    echo "  <program_name>    Name of the program to run"
    echo "  [<geninput_args>] Additional arguments for geninput.py"
    echo ""
    echo "  Generator script arguments:"
    echo "    -l <num_rows>   Specify the number of rows for the generated data (default: 10)"
    echo "    -n <num_bits>   Specify the number of bits for each number (default: 32)"
    exit 1
}

# Parse options using getopts
while getopts "gc" opt; do
    case $opt in
        g) generate=true ;;
        c) copy=true ;;
        *) usage ;;
    esac
done

shift $((OPTIND - 1))

program_name=$1

if [ -z "$program_name" ]; then
    usage
fi
if [ "$generate" = false ] && [ "$copy" = false ]; then
    echo "At least one of -g or -c must be specified."
    usage
fi

shift

if [ "$generate" = true ]; then
    python scripts/geninput.py -e $program_name $@
    echo "Generated data for <$program_name>"
fi

if [ "$copy" = true ]; then
    cp $generated_data_dir/$program_name/* $destination_dir/
    echo "Copied <$program_name> data to $destination_dir."
fi
