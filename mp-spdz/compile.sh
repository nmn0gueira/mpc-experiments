#!/bin/bash

set -e

python_script=$1
MP_SPDZ_PATH="MP-SPDZ"

if [ -z "$python_script" ]; then
    echo "Usage: $0 <python_script> <protocol_options> <program_args>"
    exit 1
fi

# This shift is to remove the first argument (the python script name)
shift

cp src/${python_script} $MP_SPDZ_PATH/${python_script}

cd $MP_SPDZ_PATH

start_time=$(date +%s.%N)

python $python_script $@

end_time=$(date +%s.%N)
execution_time=$(echo "$end_time - $start_time" | bc)
echo "Compilation time: $execution_time seconds"