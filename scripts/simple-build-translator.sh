#!/bin/sh

mkdir -p ../build/src
mkdir -p ../bin
cd ../build/src
git clone https://github.com/mozilla/translations/
cd translations
git checkout 20d1074529279afabbe546efe013f8e1fe80ee24
cd inference
git submodule update --init --recursive
cp ../3rd_party/marian-dev/Doxyfile.in marian-fork/
#mkdir build
#cd build
#cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ../
#cd ..
#rm -rf build
#cd ..
#git apply  ../../../scripts/translations.patch
#cd marian-fork
#git apply ../../../../../scripts/translations.patch
#cd inference 
#mkdir build
#cd build
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5  \
-DCMAKE_PREFIX_PATH=$(readlink -f ../../../../) \
-DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../../) \
-DUSE_STATIC_LIBS=off \
-DBUILD_SHARED_LIBS=on \
-DCMAKE_BUILD_TYPE=Release \
-DCOMPILE_CPU=on \
-DCOMPILE_CUDA=off \
..
make
cp libmarian.so ../../../../../bin/
cp src/translator/libbergamot-translator-source.so ../../../../../bin/
#	-DUSE_RUY=on \
#-DCBLAS_LIBRARIES=$(readlink -f ../../../../lib64/libcblas.a) \
#-DTcmalloc_LIBRARIES=$(readlink -f ../../../../lib/libtcmalloc_minimal.so) \
#-DTcmalloc_INCLUDE_DIR=$(readlink -f ../../../../include) \
#-DUSE_RUY=ON \


