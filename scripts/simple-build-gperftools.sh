#!/bin/sh

mkdir -p ../build/src
cd ../build/src
git clone https://github.com/gperftools/gperftools
cd ./gperftools
git checkout gperftools-2.17.2
./autogen.sh
./configure --prefix=$(readlink -f ../../)
make
make install
cd ../../
