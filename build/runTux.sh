#!/bin/sh

export DIRNAME="$(dirname "$(readlink -f "$0")")"
export MACHINE_TYPE=`uname -m`
export SYSTEM_LD_LIBRARY_PATH="$LD_LIBRARY_PATH"

export SUPERTUXKART_DATADIR="$DIRNAME"
export SUPERTUXKART_ASSETS_DIR="$DIRNAME/data/"

cd "$DIRNAME"

if [ $MACHINE_TYPE = "x86_64" ]; then
    echo "Running 64-bit version..."
    export LD_LIBRARY_PATH="$DIRNAME/lib-64:$LD_LIBRARY_PATH"
    LD_PRELOAD=~/enhancerBin/librepeater.so "$DIRNAME/bin-64/supertuxkart" "$@"
else
    echo "Running 32-bit version..."
    export LD_LIBRARY_PATH="$DIRNAME/lib:$LD_LIBRARY_PATH"
    "$DIRNAME/bin/supertuxkart" "$@"
fi
