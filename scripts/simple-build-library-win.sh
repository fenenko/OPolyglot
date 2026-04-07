#!/bin/sh
mkdir -p ../build/mingw64
mkdir -p ../build/src
cd ../build/src
BUILD_ARCH="amd64"



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
if [ ! -f "../mingw64/include/pcre2.h" ]; then
	git clone https://github.com/PCRE2Project/pcre2
	cd pcre2
	./autogen.sh 
	mkdir build-win
	cd build-win
	../configure --prefix=$(readlink -f ../../../mingw64) --host=x86_64-w64-mingw32 --build=x86_64-linux
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
	mkdir build-mingw64
	cd build-mingw64
# with winHTTP
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DwxUSE_LIBPNG=sys -DwxUSE_ZLIB=sys -DwxUSE_LIBTIFF=sys ../
	make
	make install
	cd ../../
fi

if [ ! -f "../mingw64/include/tommath.h" ]; then
	git clone https://github.com/libtom/libtommath
	cd libtommath
	git checkout v1.3.0
	mkdir build-mingw64
	cd build-mingw64
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DBUILD_SHARED_LIBS=ON  ../
	make
	make install
	cd ../..
fi
if [ ! -f "../mingw64/include/tomcrypt.h" ]; then
	git clone https://github.com/libtom/libtomcrypt
	cd libtomcrypt
	git checkout v1.18.2
	mkdir build-mingw64
	cd build-mingw64
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DBUILD_SHARED_LIBS=ON  ../
	make 
	make install
	cd ../../
fi
if [ ! -f "../mingw64/include/leptonica/allheaders.h" ]; then
	git clone https://github.com/DanBloomberg/leptonica
	cd leptonica
	mkdir build-mingw64
	cd build-mingw64
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF ../
	make
	make install
	cd ../../
fi
if [ ! -f "../mingw64/include/tesseract/baseapi.h" ]; then
	git clone https://github.com/tesseract-ocr/tesseract
	cd tesseract
	git checkout 5.5.2
	./autogen.sh
	mkdir build
	cd build
	LEPTONICA_CFLAGS="-I$(readlink -f ../../../mingw64/include/leptonica)" LEPTONICA_LIBS="-L$(readlink -f ../../../mingw64/lib) -lleptonica.dll" ../configure --disable-debug --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
	make 
	make install
	cd ../../
fi
if [ ! -f "../mingw64/include/openblas/cblas.h" ]; then
	git clone https://github.com/OpenMathLib/OpenBLAS
	cd OpenBLAS
	git checkout v0.3.29
	mkdir build
	cd build
	cmake  -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release  -DDYNAMIC_ARCH=0 -DBINARY=64 -DNO_AVX=1 -DNO_AVX2=1 -DUSE_THREAD=0 -DNO_AFFINITY=1 -DTARGET=CORE2 -DBUILD_SHARED_LIBS=ON -DNOFORTRAN=1 -DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../mingw64) ../
	make 
	make install
	cd ../../
fi
if [ ! -f "../mingw64/bin/libmarian.dll" ]; then
	git clone https://github.com/mozilla/translations/
	cd translations
	git submodule update --init --recursive
	git apply ../../../patch/translations.mingw.patch
	mkdir build
	cd build
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../
	make
	cp inference/marian-fork/src/libmarian.dll ../../../../bin
	cp inference/src/translator/libbergamot-translator-source.dll ../../../../bin
	cp inference/marian-fork/src/libmarian.dll ../../../mingw64/bin
	cp inference/src/translator/libbergamot-translator-source.dll ../../../mingw64/bin
	cp libmarian.dll.a	../../../mingw64/lib
	cp inference/src/translator/libbergamot-translator-source.dll.a ../../../mingw64/lib
	cd ..
	cp -r inference/ ../../mingw64/include
fi
