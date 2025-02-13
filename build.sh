#!/bin/bash
cmake --build .build --target clangformat && cmake --build .build -j`nproc` && cmake --build .build --target test && cp .build/compile_commands.json .
