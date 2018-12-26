#!/usr/bin/env bash

jobs=$(nproc --all)
# Release
type=RelWithDebInfo

while getopts ':j:d' option
do
    case $option in
        j) jobs=$OPTARG;;
        d) type=Debug;;
        \? ) echo "Unknown option" >&2; exit 1;;
    esac
done
shift $((OPTIND -1))

rm -rf build
mkdir build
cd build
# set it to Debug for debugging
cmake .. -DCMAKE_BUILD_TYPE=${type}
make -j ${jobs}
