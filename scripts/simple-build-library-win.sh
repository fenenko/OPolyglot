#!/bin/sh
mkdir -p ../build/mingw64
mkdir -p ../build/src
cd ../build/src
wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.10/wxWidgets-3.2.10.tar.bz2
tar -xvf wxWidgets-3.2.10.tar.bz2
cd wxWidgets-3.2.10
mkdir build-win
cd build-win
../configure --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64) --enable-unicode --enable-shared --with-msw --disable-debug --enable-monolithic
make
make install
cd ../../
git clone https://github.com/libtom/libtommath
cd libtommath
git checkout v1.3.1-rc1
git apply ../../../scripts/libtommath.mingw.patch
make -f makefile.mingw
make -f makefile.mingw install
cd ..
git clone https://github.com/libtom/libtomcrypt
cd libtomcrypt
git checkout v1.18.2
git apply ../../../libtomcrypt.mingw.patch
make -f makefile.mingw
make -f makefile.mingw install
cd ..
git clone https://github.com/DanBloomberg/leptonica
cd leptonica
git apply ../../../scripts/leptonica.mingw.patch
mkdir build-win
cd build-win
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF
make
make install
cd ../../
wget https://download.osgeo.org/libtiff/tiff-4.7.1.tar.gz
tar -xvf tiff-4.7.1.tar.gz
cd tiff-4.7.1/
mkdir build-win
cd build-win
../configure --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
make
make install
cd ../../
git clone https://github.com/tesseract-ocr/tesseract
cd tesseract
git checkout 5.5.2
./autogen.sh
mkdir build
cd build
LEPTONICA_CFLAGS="-I$(readlink -f ../../../mingw64/include/leptonica)" LEPTONICA_LIBS="-L$(readlink -f ../../../mingw64/lib) -lleptonica.dll" ../configure --disable-debug --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
make 
make install


