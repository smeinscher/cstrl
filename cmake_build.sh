#!/bin/bash

if [[ -z "$1" ]]; then
  set "Debug"
fi
mkdir build-${1,};
cd build-${1,};
# cmake -G Ninja -DCMAKE_C_COMPILER="clang" -DCMAKE_BUILD_TYPE="$1" -DRENDER_API="OPENGL" ..;
cmake -G Ninja -DCMAKE_C_COMPILER="clang" -DCMAKE_BUILD_TYPE="$1" -DRENDER_API="VULKAN" ..;
ninja;

cd ..;
