cd server
cmake -B build -S .
cmake --build build
cd -

./server/build/servers/msva/msva server/servers/msva/sample.ini
