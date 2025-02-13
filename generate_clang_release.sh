#!/bin/bash
if [ ! -f .build/ ]; then
	rm -rf .build/
fi
mkdir .build
CC=clang CXX=clang++ cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-stdlib=libc++" -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" -B .build -S .
