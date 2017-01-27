#/bin/bash
#Compile and run the client using Makefile. If QT is not installed compilation will fail and the already compiled client will be runned.
make;
if [ -f CN_client ]; then
    echo "Compilation successful. Running client..."
    ./CN_client
else
    echo "Compilation failed. Please install QT. Running precompiled client..."
    ./main_cn_client.bin
fi
