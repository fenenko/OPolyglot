#!/bin/sh

mkdir -p ../build/src
mkdir -p ../build/linux/bin
mkdir -p ../build/linux/lib
mkdir -p ../build/linux/include
cd ../build/src

echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./ltm-1.3.0.tar.xz" ]; then
	wget -nv https://github.com/libtom/libtommath/releases/download/v1.3.0/ltm-1.3.0.tar.xz
	tar -xf ltm-1.3.0.tar.xz
fi
cd libtommath-1.3.0
mkdir build-linux
cd build-linux
cmake -DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../linux) -DBUILD_SHARED_LIBS=ON  ../
echo "Build ltm $(date)"
make 
make install
cd ../
rm -rf build-linux
cd ..
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./crypt-1.18.2.tar.xz" ]; then
	wget -nv https://github.com/libtom/libtomcrypt/releases/download/v1.18.2/crypt-1.18.2.tar.xz
	tar -xf crypt-1.18.2.tar.xz
	cd libtomcrypt-1.18.2
else
	cd libtomcrypt-1.18.2
fi
echo "Build crypt $(date)"
INSTALL_DIR="$(readlink -f ../../linux)"
# Варіант 1: Збірка СТАТИЧНОЇ бібліотеки (libtomcrypt.a)
make CC=gcc CFLAGS="-O2 -Wall -fPIC" 
make install PREFIX="$INSTALL_DIR"
make clean
cd ..
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./1.87.0.tar.gz" ]; then
	wget -nv https://github.com/DanBloomberg/leptonica/archive/refs/tags/1.87.0.tar.gz
	tar -xf 1.87.0.tar.gz
fi
cd leptonica-1.87.0
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../../linux -DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF ../
echo "Build leptonica $(date)"
make 
make install
cd ../
rm -rf build
cd ../
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./5.5.2.tar.gz" ]; then
	wget -nv https://github.com/tesseract-ocr/tesseract/archive/refs/tags/5.5.2.tar.gz
	tar -xf 5.5.2.tar.gz
fi
cd tesseract-5.5.2
./autogen.sh
mkdir build-linux
cd build-linux
LEPTONICA_CFLAGS="-I$(readlink -f ../../../linux/include/leptonica)" LEPTONICA_LIBS="-L$(readlink -f ../../../linux/lib) -lleptonica" ../configure --disable-debug --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../linux)
echo "Build tesseract $(date)"
make 
make install
cd ../
rm -rf build-linux
cd ..
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./OpenBLAS-0.3.32.tar.gz" ]; then
	wget -nv https://github.com/OpenMathLib/OpenBLAS/releases/download/v0.3.32/OpenBLAS-0.3.32.tar.gz
	tar -xf OpenBLAS-0.3.32.tar.gz
fi
cd OpenBLAS-0.3.32
mkdir build-linux
cd build-linux
cmake  -DCMAKE_BUILD_TYPE=Release  -DDYNAMIC_ARCH=1 -DBINARY=64 -DBUILD_SHARED_LIBS=ON -DNOFORTRAN=1 -DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../linux) ../
echo "Build OpenBLAS $(date)"
make 
make install
cd ../
rm -rf build-linux
cd ..
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

git clone https://github.com/mozilla/translations/
cd translations
git checkout c458f2fcb6dd6f890d92ff8272b548a35d1e5c64
git submodule update --init --recursive
git apply ../../../patch/translations.patch
mkdir build
cd build
cp -r ../inference/ ../../../linux/include
cmake  -DSSPLIT_USE_INTERNAL_PCRE2=ON -DCMAKE_BUILD_TYPE=Release ../
echo "Build translations $(date)"
make 
cp libmarian.so ../../../linux/lib/
cp inference/src/translator/libbergamot-translator-source.so ../../../linux/lib/
cd ..
rm -rf build
cd ../
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./openssl-3.6.2.tar.gz" ]; then
	wget -nv https://github.com/openssl/openssl/releases/download/openssl-3.6.2/openssl-3.6.2.tar.gz
	tar -xf openssl-3.6.2.tar.gz
fi
cd openssl-3.6.2
./Configure linux-x86_64 \
	--prefix="$(readlink -f ../../linux)" \
	--openssldir="$(readlink -f ../../linux)" \
    shared \
    no-unit-test \
   	no-idea \
	no-tests \
	-fPIC \
	--libdir=lib \
   	-static-libgcc
echo "Build openssl $(date)"
make -j$(nproc)  
make install_sw
cd ../
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./libpsl-0.21.5.tar.lz" ]; then
	wget -nv https://github.com/rockdaboot/libpsl/releases/download/0.21.5/libpsl-0.21.5.tar.lz
	tar -xf libpsl-0.21.5.tar.lz
fi
cd libpsl-0.21.5
mkdir build
cd build
CFLAGS="-fPIC" ../configure --build=x86_64-linux-gnu --prefix=$(readlink -f ../../../linux) \
	--enable-static \
	--disable-shared \
	--disable-idn \
	--disable-runtime \
	--enable-builtin
echo "Build libpsl $(date)"
make  
make install
cd ../
rm -rf build
cd ../
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"


if [ ! -f "./curl-8.19.0.tar.xz" ]; then
	wget -nv https://github.com/curl/curl/releases/download/curl-8_19_0/curl-8.19.0.tar.xz
	tar -xf curl-8.19.0.tar.xz
fi
cd curl-8.19.0
mkdir build
cd build
cmake \
	-DCMAKE_INSTALL_PREFIX="$(readlink -f ../../../linux)" \
	-DOPENSSL_ROOT_DIR="$(readlink -f ../../../linux)" \
	-DOPENSSL_CRYPTO_LIBRARY="$(readlink -f ../../../linux/lib/libcrypto.so)" \
    -DOPENSSL_SSL_LIBRARY="$(readlink -f ../../../linux/lib/libssl.so)" \
   	-DBUILD_SHARED_LIBS=ON \
    -DCURL_USE_MBEDTLS=OFF \
   	-DCURL_USE_OPENSSL=ON \
	../
echo "Build curl $(date)"
make  
make install
cd ../
rm -rf build
cd ../
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ ! -f "./wxWidgets-3.2.10.tar.bz2" ]; then
	wget -nv https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.10/wxWidgets-3.2.10.tar.bz2
	tar -xf wxWidgets-3.2.10.tar.bz2
fi
cd wxWidgets-3.2.10
mkdir build-linux
cd    build-linux
cmake -DCMAKE_INSTALL_PREFIX=../../../linux -DwxUSE_MEDIACTRL=OFF -DwxUSE_LIBPNG=sys -DwxUSE_ZLIB=sys -DwxUSE_LIBTIFF=sys -DwxBUILD_MONOLITHIC=true \
	-DwxUSE_LIBSDL=OFF -DwxUSE_SOUND=OFF -DwxUSE_JOYSTICK=OFF \
	-DwxUSE_WEBREQUEST=OFF \
	../
echo "Build wxWidgets $(date)"
make  
make install
cd ../
rm -rf build-linux
cd ../
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ -f "/workspace/build/linux/lib/pkgconfig/gio-2.0.pc" ]; then
	echo "CONFIGURE NEW GLIBC"
	export PKG_CONFIG_PATH=/workspace/build/linux/lib/pkgconfig:$PKG_CONFIG_PATH
fi
git clone https://github.com/flatpak/libportal
cd libportal
git checkout 2179c6427fc7b07787220f3f405e45af822eebf7
meson setup build \
	--prefix=$(pwd)/../../linux \
	--libdir=lib \
	-Dvapi=false \
    -Ddocs=false \
    -Dbackend-gtk3=enabled \
    -Dbackend-gtk4=disabled \
    -Dbackend-qt5=disabled \
    -Dbackend-qt6=disabled \
    -Dtests=false \
	-Dintrospection=false \
	-Dpkgconfig.relocatable=true
echo "Build libportal $(date)"
ninja -C build 
ninja -C build install
cd ..
cd ..
