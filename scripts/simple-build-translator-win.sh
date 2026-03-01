#!/bin/sh

mkdir -p ../build/src
cd ../build
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-openblas-0.3.31-1-any.pkg.tar.zst
tar -vxf mingw-w64-x86_64-openblas-0.3.31-1-any.pkg.tar.zst
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-pcre2-10.47-1-any.pkg.tar.zst
tar -vxf mingw-w64-x86_64-pcre2-10.47-1-any.pkg.tar.zst
cd ./src
git clone https://github.com/mozilla/translations/
cd translations
git submodule update --init --recursive
git apply ../../../scripts/translations.mingw64.patch
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../
make
