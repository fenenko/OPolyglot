#!/bin/sh
mkdir -p ../build/mingw64
mkdir -p ../build/src
cd ../build/src
git clone https://github.com/gperftools/gperftools
cd ./gperftools
git checkout gperftools-2.17.2
./autogen.sh
cd ../build/src/gperftools
export LIBRARY_PATH=/usr/lib/mingw64-toolchain/x86_64-w64-mingw32/lib/
export C_INCLUDE_PATH=/usr/lib/mingw64-toolchain/x86_64-w64-mingw32/include/
autoreconf -i
./configure --host=x86_64-w64-mingw32 --prefix=$(readlink -f ../../mingw64) CXX=x86_64-w64-mingw32-g++ CC=x86_64-w64-mingw32-gcc
make
make install
#cd ../../
