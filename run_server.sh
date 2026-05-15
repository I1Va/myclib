cd server
cmake -B build -S .
cmake --build build
cd -

./bardak-server/build/servers/msva/msva bardak-server/servers/msva/sample.ini
