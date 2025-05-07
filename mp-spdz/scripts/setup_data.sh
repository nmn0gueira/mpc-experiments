#!/bin/bash

set -e

generate=false
copy=false
destination_dir="MP-SPDZ/Player-Data"
generated_data_dir="data/"

usage() {
    echo "Usage: $0 [-g] [-c] <program_name>"
    echo "  -g                Generate data"
    echo "  -c                Copy generated data"
    echo "  <program_name>    Name of the program to run"
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

if [ "$generate" = true ]; then
    echo "Generated data for <$program_name>"
fi

if [ "$copy" = true ]; then
    cp $generated_data_dir/$program_name/* $destination_dir/
    echo "Copied <$program_name> data to $destination_dir."
fi
