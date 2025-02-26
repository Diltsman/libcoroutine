#!/bin/bash
if [ ! -f .build/ ]; then
	rm -rf .build/
fi
mkdir .build
CC=gcc CXX=g++ cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON -B .build -S .
