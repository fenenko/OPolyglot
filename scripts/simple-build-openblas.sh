#!/bin/sh

mkdir -p ../build/src
cd ../build/src
git clone https://github.com/OpenMathLib/OpenBLAS
git checkout v0.3.30
cd OpenBLAS/
make DYNAMIC_ARCH=1 CBLAS=1
make install PREFIX=$(readlink -f ../../)
cd ../../
