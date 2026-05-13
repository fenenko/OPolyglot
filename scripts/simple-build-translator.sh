#!/bin/sh

mkdir -p ../build/src
mkdir -p ../build/linux/bin
mkdir -p ../build/linux/lib
mkdir -p ../build/linux/include
mkdir -p ../bin
cd ../build/src
if [ ! -f "./1.87.0.tar.gz" ]; then
	wget https://github.com/DanBloomberg/leptonica/archive/refs/tags/1.87.0.tar.gz
	tar -xf 1.87.0.tar.gz
fi
cd leptonica-1.87.0
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../../linux -DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF ../
make 
make install
cd ../
rm -rf build
cd ../
if [ ! -f "./5.5.2.tar.gz" ]; then
	wget -nv https://github.com/tesseract-ocr/tesseract/archive/refs/tags/5.5.2.tar.gz
	tar -xf 5.5.2.tar.gz
	ls ./
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
git clone https://github.com/mozilla/translations/
cd translations
git submodule update --init --recursive
git apply ../../../patch/translations.patch
echo "----------------------------"
echo "----------------------------"
echo "----------------------------"
ls ../../../
ls ../../../patch
mkdir build
cd build
cp -r ../inference/ ../../../linux/include
cmake ..
make
cp libmarian.so ../../../../bin/
cp inference/src/translator/libbergamot-translator-source.so ../../../../bin/
cp libmarian.so ../../../linux/bin/
cp inference/src/translator/libbergamot-translator-source.so ../../../linux/bin/
cd ..
