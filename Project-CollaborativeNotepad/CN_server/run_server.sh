#!/bin/bash
# Compile and run main_cn_server.cpp
g++ main_cn_server.cpp -pthread -std=gnu++11 -o main_cn_server.bin && ./main_cn_server.bin
