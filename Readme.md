# Simple Messenger
This is a simple 1 to 1 messenger program using linux tcp sockets. It includes the main program `simple-messenger` which runs a messenger in a console, but the target `libmessenger.a` is a library that provides further expansion if wanted.

## Building
This was built using cmake, there are also some tests that can be made optionally using the cmake option `BUILD_TESTS=ON`, know that gtest needs to be installed on your system for that to compile properly. Here are the steps below for building with no tests from the root of this repository:
```
mkdir build
cd build
cmake ..
cmake --build .
```

## Running
`simple-messenger` can be ran in two modes, server or client. A `simple-messenger` in server mode needs to be running for a client to work.  
To run in server mode supply the executable no arguments:
```
./simple-messenger
```
The client needs an argument for the address of you server, e.g. if you were connecting to a server on the same computer you would run this:
```
./simple-messenger 127.0.0.1
```
The server always runs on port 1997 unless the preprocessor definition `SERVER_PORT` is set to a different number when compiling.