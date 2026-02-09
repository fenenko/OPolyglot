#!/bin/sh

mkdir -p ../build/src
mkdir -p ../bin
cd ../build/src
git clone https://github.com/mozilla/translations/
cd translations
cd inference
git submodule update --init --recursive
git checkout -b opolyglot 20d1074529279afabbe546efe013f8e1fe80ee24
cp ../3rd_party/marian-dev/Doxyfile.in marian-fork/
git apply ../../../../scripts/inference.patch
#git add CMakeLists.txt src/translator/CMakeLists.txt marian-fork/src/CMakeLists.txt marian-fork/Doxyfile.in
#git commit -m "configure for build shared library"
mkdir build
cd build
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5  \
	-DUSE_STATIC_LIBS=off \
	-DBUILD_SHARED_LIBS=on \
	-DCMAKE_BUILD_TYPE=Release \
	-DCOMPILE_CPU=on \
	-DCOMPILE_CUDA=off \
	-DSSPLIT_USE_INTERNAL_PCRE2=on \
	-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
	-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
	..
make
cp libmarian.so ../../../../../bin/
cp src/translator/libbergamot-translator-source.so ../../../../../bin/


#	-DCMAKE_PREFIX_PATH=$(readlink -f ../../../../) \
#	-DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../../) \
