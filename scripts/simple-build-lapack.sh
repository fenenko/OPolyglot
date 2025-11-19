#!/bin/sh


mkdir -p ../build/src

cd ../build/src
git clone https://github.com/Reference-LAPACK/lapack/
cd lapack
git checkout v3.12.1
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../)  ../ -DCBLAS=ON
make
make install
#cd ../../
