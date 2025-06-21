#!/bin/bash

set -e

python_script=$1
MP_SPDZ_PATH="MP-SPDZ"

if [ -z "$python_script" ]; then
    echo "Usage: $0 <python_script> <protocol_options> <program_args>"
    exit 1
fi

shift

cp src/${python_script} $MP_SPDZ_PATH/${python_script}

cd $MP_SPDZ_PATH

start_time=$(date +%s%N)
python3 $python_script $@
end_time=$(date +%s%N)
elapsed_time=$(awk "BEGIN {print ($end_time - $start_time) / 1000000000}")
echo "Compilation time: ${elapsed_time} seconds"