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

python3 extract_stdlib.py client/build/combined.elf stdlib.bin

# cd -

# # rm ./../back-end/lib/stdlib.bin
# mv stdlib/stdlib.bin ./../back-end/lib/stdlib.bin


# echo "---running client---"
# ./client/build/client localhost 3000