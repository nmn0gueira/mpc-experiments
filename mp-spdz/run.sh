#!/bin/bash

set -ex

protocol_script=$1
program_name=$2
MP_SPDZ_PATH="MP-SPDZ"

if [ -z "$protocol_script" ] || [ -z "$program_name" ]; then
    echo "Usage: $0 <protocol_script> <program_name> [<args>]"
    exit 1
fi

# Shift the first two arguments
shift 2

cd $MP_SPDZ_PATH

Scripts/${protocol_script} $program_name "$@" || {
    echo "Failed to run the protocol script"
    exit 1
}