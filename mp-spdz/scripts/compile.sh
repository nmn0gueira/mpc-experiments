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

/usr/bin/time -f "Compilation time: %e seconds" python $python_script $@