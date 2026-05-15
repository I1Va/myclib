#!/bin/bash
set -e  # exit on error

echo "---building stdlib.bin---"

cd libs
mkdir -p build && cd build
cmake ..
make combined_elf
cd ./../../.

python3 extract_stdlib.py libs/build/combined.elf stdlib.bin

# cd -

# # rm ./../back-end/lib/stdlib.bin
# mv stdlib/stdlib.bin ./../back-end/lib/stdlib.bin


# echo "---running client---"
# ./client/build/client localhost 3000