#!/bin/sh
mkdir -p ../build/mingw64
mkdir -p ../build/src
cd ../build/src


if [ ! -f "../mingw64/include/zlib.h" ]; then
	wget https://www.zlib.net/zlib-1.3.2.tar.gz
	tar -xvf zlib-1.3.2.tar.gz
	cd ./zlib-1.3.2
	mkdir build-win
	cd build-win
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake \
									-DCMAKE_INSTALL_PREFIX=../../../mingw64 \
									-DZLIB_BUILD_STATIC=OFF \
									../
	make
	make install
	cd ../../
fi
if [ ! -f "../mingw64/include/png.h" ]; then
	wget https://download.sourceforge.net/libpng/libpng-1.6.56.tar.xz
	tar -xvf libpng-1.6.56.tar.xz
	cd ./libpng-1.6.56
	mkdir build-win
	cd build-win
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DPNG_STATIC=OFF ../
	make
	make install
	cd ../../
fi
if [ ! -f "../mingw64/include/tiff.h" ]; then
	wget https://download.osgeo.org/libtiff/tiff-4.7.1.tar.gz
	tar -xvf tiff-4.7.1.tar.gz
	cd tiff-4.7.1/
	mkdir build-win
	cd build-win
	../configure --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
	make
	make install
	cd ../../
fi
if [ ! -f "../mingw64/include/wx-3.2/wx/wx.h" ]; then
	wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.10/wxWidgets-3.2.10.tar.bz2
	tar -xvf wxWidgets-3.2.10.tar.bz2
	cd wxWidgets-3.2.10
	mkdir build-win
	cd build-win
	../configure --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64) --enable-unicode --enable-shared --with-msw --disable-debug --enable-monolithic
	make
	make install
	cd ../../
fi

if [ ! -f "../mingw64/include/tommath.h" ]; then
	git clone https://github.com/libtom/libtommath
	cd libtommath
	git checkout v1.3.0
	mkdir build-win
	cd build-win
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DBUILD_SHARED_LIBS=ON  ../
	make
	make install
	cd ../..
fi
if [ ! -f "../mingw64/include/tomcrypt.h" ]; then
	git clone https://github.com/libtom/libtomcrypt
	cd libtomcrypt
	git checkout v1.18.2
	git apply ../../../patch/libtomcrypt.mingw.patch
	make -f makefile.mingw CC=x86_64-w64-mingw32-gcc CFLAGS="-U_FORTIFY_SOURCE -O2 -Wall"
	make -f makefile.mingw install
	cd ..
fi
if [ ! -f "../mingw64/include/leptonica/allheaders.h" ]; then
	git clone https://github.com/DanBloomberg/leptonica
	cd leptonica
	git apply ../../../patch/leptonica.mingw.patch
	mkdir build-win
	cd build-win
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF ../
	make
	make install
	cd ../../
fi
git clone https://github.com/tesseract-ocr/tesseract
cd tesseract
git checkout 5.5.2
./autogen.sh
mkdir build
cd build
LEPTONICA_CFLAGS="-I$(readlink -f ../../../mingw64/include/leptonica)" LEPTONICA_LIBS="-L$(readlink -f ../../../mingw64/lib) -lleptonica.dll" ../configure --disable-debug --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
make 
make install


