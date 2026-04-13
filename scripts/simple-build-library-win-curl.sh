#!/bin/sh
mkdir -p ../build/mingw64
mkdir -p ../build/src
cd ../build/src
BUILD_ARCH="amd64"
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
