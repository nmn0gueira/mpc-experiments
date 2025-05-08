#!/bin/bash

set -e

fromsource=$1
MP_SPDZ_VERSION="0.4.0" # Version 0.4.0 is impossible to build from source. This was fixed at a later date. Checkout used was 94c4e2791528b036b981941a418e541a6a243962

if [ "$fromsource" = "yes" ]; then
    # this may take a long time
    git clone https://github.com/data61/MP-SPDZ.git
    cd MP-SPDZ

    make setup
    make all -j
else
    curl -L https://github.com/data61/MP-SPDZ/releases/download/v$MP_SPDZ_VERSION/mp-spdz-$MP_SPDZ_VERSION.tar.xz | tar xJv
    mv mp-spdz-$MP_SPDZ_VERSION MP-SPDZ
    cd MP-SPDZ
    Scripts/tldr.sh
fi