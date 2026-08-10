#!/bin/sh


# Ініціалізація прапорців (за замовчуванням збираємо все)
BUILD_CURL=1
BUILD_LIBPORTAL=1
BUILD_TESSERACT=1
BUILD_WXWIDGETS=1
BUILD_BERGAMOT=1

# Обробка опцій командного рядка
while [ "$#" -gt 0 ]; do
    case "$1" in
        -help|--help)
            echo "Використання: $0 [ОПЦІЇ]"
            echo "Доступні опції:"
            echo "  -help          Показати цю довідку та вийти"
            echo "  -no-curl       Пропустити завантаження та збирання curl"
            echo "  -no-libportal  Пропустити завантаження та збирання libportal"
            echo "  -no-tesseract  Пропустити завантаження та збирання tesseract"
            echo "  -no-wx		   Пропустити завантаження та збирання wxWidgets"
            echo "  -no-bergamot   Пропустити завантаження та збирання bergamot"
            exit 0
            ;;
        -no-curl)
            BUILD_CURL=0
            shift
            ;;
        -no-libportal)
            BUILD_LIBPORTAL=0
            shift
            ;;
        -no-tesseract)
            BUILD_TESSERACT=0
            shift
            ;;
        -no-wx)
            BUILD_WXWIDGETS=0
            shift
            ;;
        -no-bergamot)
            BUILD_BERGAMOT=0
            shift
            ;;
        *)
            echo "Невідома опція: $1"
            echo "Спробуйте '$0 -help' для отримання списку доступних опцій."
            exit 1
            ;;
    esac
done

mkdir -p ../build/src
mkdir -p ../build/linux/bin
mkdir -p ../build/linux/lib
mkdir -p ../build/linux/include
cd ../build/src

if [ "$BUILD_CURL" -eq 1 ]; then
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
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DOPENSSL_ROOT_DIR="$(readlink -f ../../../linux)" \
	-DOPENSSL_CRYPTO_LIBRARY="$(readlink -f ../../../linux/lib/libcrypto.so)" \
    -DOPENSSL_SSL_LIBRARY="$(readlink -f ../../../linux/lib/libssl.so)" \
   	-DBUILD_SHARED_LIBS=ON \
    -DCURL_USE_MBEDTLS=OFF \
   	-DCURL_USE_OPENSSL=ON \
	../
echo "Build curl $(date)"
make -j$(nproc) 
make install
cd ../
rm -rf build
cd ../
fi
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

if [ "$BUILD_WXWIDGETS" -eq 1 ]; then
if [ ! -f "./wxWidgets-3.2.10.tar.bz2" ]; then
	wget -nv https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.10/wxWidgets-3.2.10.tar.bz2
	tar -xf wxWidgets-3.2.10.tar.bz2
fi
cd wxWidgets-3.2.10
mkdir build-linux
cd    build-linux
cmake -DCMAKE_INSTALL_PREFIX=../../../linux -DwxUSE_MEDIACTRL=OFF -DwxBUILD_MONOLITHIC=true \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DwxUSE_LIBSDL=OFF -DwxUSE_SOUND=OFF -DwxUSE_JOYSTICK=OFF \
	-DwxUSE_WEBREQUEST=OFF \
	../
echo "Build wxWidgets $(date)"
make -j$(nproc) 
make install
cd ../
rm -rf build-linux
cd ../
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"
fi


if [ "$BUILD_TESSERACT" -eq 1 ]; then
if [ ! -f "./1.87.0.tar.gz" ]; then
	wget -nv https://github.com/DanBloomberg/leptonica/archive/refs/tags/1.87.0.tar.gz
	tar -xf 1.87.0.tar.gz
fi
cd leptonica-1.87.0
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../../linux	-DCMAKE_INSTALL_LIBDIR=lib \
	-DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF ../
echo "Build leptonica $(date)"
make -j$(nproc)
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
LEPTONICA_CFLAGS="-I$(readlink -f ../../../linux/include/leptonica)" LEPTONICA_LIBS="-L$(readlink -f ../../../linux/lib) -lleptonica" ../configure --disable-debug --build=x86_64-linux-gnu --without-curl --prefix=$(readlink -f ../../../linux)
echo "Build tesseract $(date)"
make -j$(nproc)
make install
cd ../
rm -rf build-linux
cd ..
fi


if [ "$BUILD_BERGAMOT" -eq 1 ]; then
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"

git clone https://github.com/mozilla/translations/
cd translations
git checkout c458f2fcb6dd6f890d92ff8272b548a35d1e5c64
git submodule update --init --recursive
ls ../../../
git apply ../../../patch/translations.patch
mkdir build
cd build
cp -r ../inference/ ../../../linux/include
cmake  -DSSPLIT_USE_INTERNAL_PCRE2=ON -DCOMPILE_LIBRARY_ONLY=ON -DCMAKE_BUILD_TYPE=Release ../
echo "Build translations $(date)"
make -j$(nproc)
cp libmarian.so ../../../linux/lib/
cp inference/src/translator/libbergamot-translator-source.so ../../../linux/lib/
cd ..
rm -rf build
cd ../
fi


if [ "$BUILD_LIBPORTAL" -eq 1 ]; then
echo "--------------------------------------------------"
echo "--------------------------------------------------"
echo "--------------------------------------------------"
git clone https://github.com/flatpak/libportal
cd libportal
git checkout 467a397fd7996557f837cdc26ac07c01c62810e5
meson setup build \
	--prefix=$(pwd)/../../linux \
	--libdir=lib \
	-Dbackends=gtk3 \
	-Dintrospection=false \
	-Dvapi=false \
	-Ddocs=false \
	-Dpkgconfig.relocatable=true
echo "Build libportal $(date)"
ninja -C build 
ninja -C build install
cd ..
fi

wget https://github.com/bblanchon/pdfium-binaries/releases/download/chromium%2F7906/pdfium-linux-x64.tgz
pwd
tar -xvf pdfium-linux-x64.tgz -C ../linux
