#!/bin/sh

mkdir -p ../build/src
mkdir -p ../build/linux/bin
mkdir -p ../build/linux/lib
mkdir -p ../build/linux/include
mkdir -p ../bin
cd ../build/src
git clone https://github.com/mozilla/translations/
cd translations
git submodule update --init --recursive
git apply ../../../scripts/translations.patch
mkdir build
cd build
cmake ..
make
cp libmarian.so ../../../../bin/
cp inference/src/translator/libbergamot-translator-source.so ../../../../bin/
cp libmarian.so ../../../linux/bin/
cp inference/src/translator/libbergamot-translator-source.so ../../../linux/bin/
cd ..
cp -r inference/ ../../linux/include
