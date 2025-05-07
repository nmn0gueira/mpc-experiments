#!/bin/bash

set -e

MP_SPDZ_VERSION="${1:-0.4.0}"

curl -L https://github.com/data61/MP-SPDZ/releases/download/v$MP_SPDZ_VERSION/mp-spdz-$MP_SPDZ_VERSION.tar.xz | tar xJv
mv mp-spdz-$MP_SPDZ_VERSION MP-SPDZ
cd MP-SPDZ
Scripts/tldr.sh