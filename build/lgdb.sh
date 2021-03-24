#!/bin/bash

if [ -z "$1" ]
then
    echo "USAGE: $0 <path-to-example>"
    echo "Example: $0 ./src/1.getting_started/1.2.hello_window"
    exit 1
fi

EXAMPLE_PATH=$1
EXAMPLE_NAME=`basename $1`
EXAMPLE_EXE=`find ./bin -executable  | grep "${EXAMPLE_NAME}"`
EXAMPLE_EXE_ABSOLUTE=`realpath ${EXAMPLE_EXE}`


ENHANCER_RELATIVE_DLL=`find ../../ -type f | grep "librepeater" | grep ".so"`
echo "Path to .so: ${ENHANCER_RELATIVE_DLL}"
if [ -z "${ENHANCER_RELATIVE_DLL}" ]
then
    echo "Missing enhancer .so/DLL file! Try to build Enhancer at first, then run this."
    exit 2
fi

ENHANCER_DLL=`realpath ${ENHANCER_RELATIVE_DLL}`

# START APP
cd $1
echo "Running set environment LD_PRELOAD=${ENHANCER_DLL} ${EXAMPLE_EXE_ABSOLUTE}" 
gdb -ex "set environment LD_PRELOAD=${ENHANCER_DLL}" ${EXAMPLE_EXE_ABSOLUTE} 
