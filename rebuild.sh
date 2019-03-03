#!/usr/bin/env bash

set -e

jobs=$(nproc --all)
# set it to Debug for proper debugging
type=RelWithDebInfo
ninja=false

while getopts ':j:dn' option
do
    case $option in
        j) jobs=$OPTARG;;
        d) type=Debug;;
        n) ninja=true;;
        \? ) echo "Unknown option" >&2; exit 1;;
    esac
done
shift $((OPTIND -1))

rm -rf build
mkdir build
cd build

if [ $ninja = true ]; then
    cmake .. -DCMAKE_BUILD_TYPE=${type} -GNinja
    ninja
else
    cmake .. -DCMAKE_BUILD_TYPE=${type}
    make -j ${jobs}
fi
