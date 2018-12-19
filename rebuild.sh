#!/usr/bin/env bash

rm -rf build
mkdir build
cd build
# set it to Debug for debugging
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 8
