#!/bin/sh
mkdir -p ../build/mingw64
mkdir -p ../build/src
cd ../build/src
rm -rf ../mingw64
mkdir ../mingw64
BUILD_ARCH="amd64"



if [ ! -f "./zlib-1.3.2.tar.gz" ]; then
	wget -nv https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz
	tar -xf zlib-1.3.2.tar.gz
fi
cd ./zlib-1.3.2
mkdir build-mingw64
cd build-mingw64
cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake \
						-DCMAKE_INSTALL_PREFIX=../../../mingw64 \
						-DZLIB_BUILD_STATIC=OFF \
						../
echo "Build zlib $(date)"
make > ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ../

if [ ! -f "./pcre2-10.47.tar.gz" ]; then
	wget -nv https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.47/pcre2-10.47.tar.gz
	tar -xf pcre2-10.47.tar.gz
fi
cd pcre2-10.47
./autogen.sh 
mkdir build-mingw64
cd build-mingw64
../configure --prefix=$(readlink -f ../../../mingw64) --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu
echo "Build pcre2 $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ../

if [ ! -f "./libpng-1.6.56.tar.xz" ]; then
	wget -nv https://download.sourceforge.net/libpng/libpng-1.6.56.tar.xz
	tar -xf libpng-1.6.56.tar.xz
fi
cd ./libpng-1.6.56
mkdir build-mingw64
cd build-mingw64
cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DPNG_STATIC=OFF ../
echo "Build libpng $(date)"
make  >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ../

if [ ! -f "./tiff-4.7.1.tar.gz" ]; then
	wget -nv https://download.osgeo.org/libtiff/tiff-4.7.1.tar.gz
	tar -xf tiff-4.7.1.tar.gz
fi
cd tiff-4.7.1/
mkdir build-mingw64
cd build-mingw64
../configure --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
echo "Build tiff $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ..

if [ ! -f "./wxWidgets-3.2.10.tar.bz2" ]; then
	wget -nv https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.10/wxWidgets-3.2.10.tar.bz2
	tar -xf wxWidgets-3.2.10.tar.bz2
fi
cd wxWidgets-3.2.10
mkdir build-mingw64
cd build-mingw64
cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DwxUSE_MEDIACTRL=OFF -DwxUSE_LIBPNG=sys -DwxUSE_ZLIB=sys -DwxUSE_LIBTIFF=sys -DwxUSE_WEBREQUEST=OFF ../
echo "Build wxWidgets $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ..

if [ ! -f "./leptonica-1.87.0.tar.gz" ]; then
	wget -nv https://github.com/DanBloomberg/leptonica/releases/download/1.87.0/leptonica-1.87.0.tar.gz
	tar -xf leptonica-1.87.0.tar.gz
fi
cd leptonica-1.87.0
mkdir build-mingw64
cd build-mingw64
cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_INSTALL_PREFIX=../../../mingw64 -DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF ../
echo "Build leptonica $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ..

if [ ! -f "./5.5.2.tar.gz" ]; then
	wget -nv https://github.com/tesseract-ocr/tesseract/archive/refs/tags/5.5.2.tar.gz
	tar -xf 5.5.2.tar.gz
fi
cd tesseract-5.5.2
./autogen.sh
mkdir build-mingw64
cd build-mingw64
LEPTONICA_CFLAGS="-I$(readlink -f ../../../mingw64/include/leptonica)" LEPTONICA_LIBS="-L$(readlink -f ../../../mingw64/lib) -lleptonica.dll" ../configure --disable-debug --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64)
echo "Build tesseract $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ..
if [ ! -f "./OpenBLAS-0.3.32.tar.gz" ]; then
	wget -nv https://github.com/OpenMathLib/OpenBLAS/releases/download/v0.3.32/OpenBLAS-0.3.32.tar.gz
	tar -xf OpenBLAS-0.3.32.tar.gz
fi
cd OpenBLAS-0.3.32
mkdir build-mingw64
cd build-mingw64
cmake  -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release  -DDYNAMIC_ARCH=0 -DBINARY=64 -DNO_AVX=1 -DNO_AVX2=1 -DUSE_THREAD=0 -DNO_AFFINITY=1 -DTARGET=CORE2 -DBUILD_SHARED_LIBS=ON -DNOFORTRAN=1 -DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../mingw64) ../
echo "Build OpenBLAS $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ..
echo "Download mozilla/translations $(date)"
git clone https://github.com/mozilla/translations/
cd translations
git checkout c458f2fcb6dd6f890d92ff8272b548a35d1e5c64
git submodule update --init --recursive
git apply ../../../patch/translations.mingw.patch
mkdir build-mingw64
cd build-mingw64
cmake -DCMAKE_TOOLCHAIN_FILE=../../../../scripts/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ../
echo "Build mozilla/translations $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
cp inference/marian-fork/src/libmarian.dll ../../../mingw64/bin
cp inference/src/translator/libbergamot-translator-source.dll ../../../mingw64/bin
cp libmarian.dll.a	../../../mingw64/lib
cp inference/src/translator/libbergamot-translator-source.dll.a ../../../mingw64/lib
cd ..
cp -r inference/ ../../mingw64/include
rm -rf build-mingw64
cd ../

if [ ! -f "./libpsl-0.21.5.tar.lz" ]; then
	wget -nv https://github.com/rockdaboot/libpsl/releases/download/0.21.5/libpsl-0.21.5.tar.lz
	tar -xf libpsl-0.21.5.tar.lz
fi
cd libpsl-0.21.5
mkdir build-mingw64
cd build-mingw64
../configure --host=x86_64-w64-mingw32 --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../mingw64) \
	--enable-static \
	--disable-shared \
	--disable-idn \
	--disable-runtime \
	--enable-builtin
echo "Build libpsl $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ../

if [ ! -f "./openssl-3.6.2.tar.gz" ]; then
	wget -nv https://github.com/openssl/openssl/releases/download/openssl-3.6.2/openssl-3.6.2.tar.gz
	tar -xf openssl-3.6.2.tar.gz
fi
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
	--libdir=lib \
	mingw64 \
   	-static-libgcc
echo "Build openssl $(date)"
make -j$(nproc)  >> ../../mingw64/buildlog.txt 2>&1
make install_sw
make distclean
cd ../

if [ ! -f "./curl-8.19.0.tar.xz" ]; then
	wget -nv https://github.com/curl/curl/releases/download/curl-8_19_0/curl-8.19.0.tar.xz
	tar -xf curl-8.19.0.tar.xz
fi
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
echo "Build curl $(date)"
make >> ../../../mingw64/buildlog.txt 2>&1
make install
cd ../
rm -rf build-mingw64
cd ../
