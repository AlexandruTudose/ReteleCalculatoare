#!/bin/bash
# Compile and run main_cn_server.cpp
g++ main_cn_server.cpp -pthread -std=gnu++11 -o CN_server
if [ -f CN_server ]; then
    echo "Compilation successful. Running server..."
    ./CN_server
else
    echo "Compilation failed. Please install g++. Running precompiled server..."
    ./main_cn_server.bin
fi
