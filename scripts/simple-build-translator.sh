#!/bin/sh

mkdir -p ../build/src
cd ../build/src
git clone https://github.com/mozilla/translations/
cd translations
git checkout 20d1074529279afabbe546efe013f8e1fe80ee24
cd inference
git submodule update --init --recursive
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
#cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5  \
#-DUSE_RUY=ON \
#-DCMAKE_PREFIX_PATH=$(readlink -f ../../../../) \
#-DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../../) \
#-DUSE_STATIC_LIBS=on \
#-DCOMPILE_CPU=on \
#-DCOMPILE_CUDA=off \
#../

#	-DUSE_RUY=on \
#-DCBLAS_LIBRARIES=$(readlink -f ../../../../lib64/libcblas.a) \
#-DTcmalloc_LIBRARIES=$(readlink -f ../../../../lib/libtcmalloc_minimal.so) \
#-DTcmalloc_INCLUDE_DIR=$(readlink -f ../../../../include) \

#-DBUILD_SHARED_LIBS=on \

