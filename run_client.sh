#!/bin/bash
set -e  # exit on error

echo "---building myclib---"
cd myclib
mkdir -p build
cd build
cmake ..
make
cd ../..  # back to root

echo "---building client---"
cd client
mkdir -p build
cd build
cmake ..
make
cd ../..  # back to root

echo "---running client---"
./client/build/client localhost 3000