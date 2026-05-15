cd client

cmake -B build -S .
cmake --build build
cd -

./client/build/client localhost 3000
