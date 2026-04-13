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
	../configure --prefix=$(readlink -f ../../../mingw64) --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu
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
	mkdir build-win
	cd build-win
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
	mkdir build-mingw64
	cd build-mingw64
	LEPTONICA_CFLAGS="-I$(readlink -f ../../../mingw64/include/leptonica)" LEPTONICA_LIBS="-L$(readlink -f ../../../mingw64/lib) -lleptonica.dll" ../configure --disable-debug --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
	make 
	make install
	cd ../../
fi
if [ ! -f "../mingw64/include/openblas/cblas.h" ]; then
	git clone https://github.com/OpenMathLib/OpenBLAS
	cd OpenBLAS
	git checkout v0.3.29
	mkdir build-mingw64
	cd build-mingw64
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
	mkdir build-mingw64
	cd build-mingw64
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../
	make
	cp inference/marian-fork/src/libmarian.dll ../../../mingw64/bin
	cp inference/src/translator/libbergamot-translator-source.dll ../../../mingw64/bin
	cp libmarian.dll.a	../../../mingw64/lib
	cp inference/src/translator/libbergamot-translator-source.dll.a ../../../mingw64/lib
	cd ..
	cp -r inference/ ../../mingw64/include
fi
if [ ! -f "../mingw64/lib/libpsl.a" ]; then
	wget https://github.com/rockdaboot/libpsl/releases/download/0.21.5/libpsl-0.21.5.tar.lz
	tar -xvf libpsl-0.21.5.tar.lz
	cd libpsl-0.21.5
	mkdir build-mingw64
	cd build-mingw64
	../configure --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64) \
		--enable-static \
		--disable-shared \
		--disable-idn \
		--disable-runtime \
		--enable-builtin
	make
	make install
	cd ../../
fi
if [ ! -f "../mingw64/lib/libmbedtls.dll.a" ]; then
	wget https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-4.1.0/mbedtls-4.1.0.tar.bz2
	tar -xvf mbedtls-4.1.0.tar.bz2
	cd mbedtls-4.1.0
	python3 scripts/config.py set MBEDTLS_PEM_PARSE_C
	python3 scripts/config.py set MBEDTLS_BASE64_C
	python3 scripts/config.py set MBEDTLS_ECDSA_C
	python3 scripts/config.py set MBEDTLS_ECP_C
	python3 scripts/config.py set MBEDTLS_ECP_DP_SECP256R1_ENABLED
	python3 scripts/config.py set MBEDTLS_ECP_DP_SECP384R1_ENABLED
	python3 scripts/config.py set MBEDTLS_ERROR_C
	python3 scripts/config.py set MBEDTLS_ENTROPY_C
	python3 scripts/config.py set MBEDTLS_HMAC_DRBG_C 
	python3 scripts/config.py set MBEDTLS_HAVE_PLATFORM_ENTROPY
	mkdir build-mingw64
	cd build-mingw64
	cmake  \
		-DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake \
		-DCMAKE_INSTALL_PREFIX=../../../mingw64 \
    	-DCMAKE_BUILD_TYPE=Release \
	    -DUSE_SHARED_MBEDTLS_LIBRARY=ON \
    	-DUSE_STATIC_MBEDTLS_LIBRARY=OFF \
	    -DENABLE_TESTING=OFF \
    	-DENABLE_PROGRAMS=OFF \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    	-DCMAKE_C_FLAGS="-ladvapi32 -lbcrypt" \
		..
	make 
	make install
	cd ../../
fi
if [ ! -f "../mingw64/bin/libssl-3-x64.dll" ]; then
	wget https://github.com/openssl/openssl/releases/download/openssl-3.6.2/openssl-3.6.2.tar.gz
	tar -xvf openssl-3.6.2.tar.gz
	cd openssl-3.6.2
	./Configure \
		--cross-compile-prefix=x86_64-w64-mingw32- \
		--prefix="$(readlink -f ../../mingw64)" \
		--openssldir="$(readlink -f ../../mingw64)" \
	    shared \
    	zlib \
		--with-zlib-include="$(readlink -f ../../mingw64/include)" \
		--with-zlib-lib="$(readlink -f ../../mingw64/lib)" \
	    no-unit-test \
    	no-idea \
		mingw64 \
    	-static-libgcc
	make -j$(nproc)
	make install_sw
	cd ../
fi
if [ ! -f "../mingw64/lib/libcurl.dll.a" ]; then
	wget https://curl.se/download/curl-8.19.0.tar.xz
	tar -xvf curl-8.19.0.tar.xz
	cd curl-8.19.0
	mkdir build-mingw64
	cd build-mingw64
	cmake \
		-DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake \
		-DCMAKE_INSTALL_PREFIX=../../../mingw64 \
    	-DBUILD_SHARED_LIBS=ON \
	    -DCURL_USE_MBEDTLS=OFF \
    	-DCURL_USE_OPENSSL=ON \
		..
	make 
	make install
fi
