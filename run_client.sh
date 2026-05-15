#!/bin/bash

echo "---building myclib---"
cd myclib
mkdir -p build && cd build
cmake ..
make
cd -


# cd client

# cmake -B build -S .
# cmake --build build
# cd -

# ./client/build/client localhost 3000
