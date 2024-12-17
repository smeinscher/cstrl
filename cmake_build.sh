#!/bin/bash

if [[ ! -v $1 ]]; then
  set "Debug"
fi
mkdir build-${1,};
cd build-${1,};
cmake -G Ninja -DCMAKE_C_COMPILER="C:\Program Files\LLVM\bin\clang.exe" -DCMAKE_BUILD_TYPE="$1" ..;
ninja;

cd ..;
