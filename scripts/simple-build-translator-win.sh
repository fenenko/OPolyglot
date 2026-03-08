#!/bin/sh

mkdir -p ../build/mingw64
mkdir -p ../build/src
cd ../build
cd ./src
git clone https://github.com/PCRE2Project/pcre2
cd pcre2
./autogen.sh 
mkdir build-win
cd build-win
../configure --prefix=$(readlink -f ../../../mingw64) --host=x86_64-w64-mingw32 --build=x86_64-linux
make 
make install
cd ../../
git clone https://github.com/OpenMathLib/OpenBLAS
cd OpenBLAS
git checkout v0.3.29
mkdir build
cd build
cmake  -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release  -DDYNAMIC_ARCH=0 -DBINARY=64 -DNO_AVX=1 -DNO_AVX2=1 -DUSE_THREAD=0 -DNO_AFFINITY=1 -DTARGET=CORE2 -DBUILD_SHARED_LIBS=ON -DNOFORTRAN=1 -DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../mingw64) ../
make 
make install
cd ../../
git clone https://github.com/mozilla/translations/
cd translations
git submodule update --init --recursive
git apply ../../../scripts/translations.mingw.patch
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../
make
cp inference/marian-fork/src/libmarian.dll ../../../../bin
cp inference/src/translator/libbergamot-translator-source.dll ../../../../bin
cp inference/marian-fork/src/libmarian.dll ../../../mingw64/bin
cp inference/src/translator/libbergamot-translator-source.dll ../../../mingw64/bin
cp libmarian.dll.a	../../../mingw64/lib
cp inference/src/translator/libbergamot-translator-source.dll.a ../../../mingw64/lib
cd ..
cp -r inference/ ../../mingw64/include
