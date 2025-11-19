#!/bin/sh

mkdir -p ../build/src
cd ../build/src
git clone https://github.com/mozilla/translations/
cd translations
git checkout 20d1074529279afabbe546efe013f8e1fe80ee24
cd inference
mkdir build
cd build
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ../
cd ..
rm -rf build
cd ..
git apply  ../../../scripts/translations_mingw64.patch
cd inference 
mkdir build_win
cd build_win
cmake -G "MinGW Makefiles" \
-DCMAKE_POLICY_VERSION_MINIMUM=3.5  \
-DCMAKE_SYSTEM_NAME=Windows \
-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
-DCMAKE_PREFIX_PATH=$(readlink -f ../../../../mingw64) \
-DTcmalloc_INCLUDE_DIR=$(readlink -f ../../../../mingw64/include) \
-DBUILD_SHARED_LIBS=on \
-DUSE_STATIC_LIBS=off \
-DCOMPILE_CPU=on \
-DCOMPILE_CUDA=off \
../

# -DUSE_RUY=ON \
#	-DUSE_RUY=on \
#-DCBLAS_LIBRARIES=$(readlink -f ../../../../lib64/libcblas.a) \
#-DTcmalloc_LIBRARIES=$(readlink -f ../../../../lib/libtcmalloc_minimal.so) \
#-DTcmalloc_INCLUDE_DIR=$(readlink -f ../../../../include) \
#-DCMAKE_PREFIX_PATH=$(readlink -f ../../../../) \
#-DCMAKE_INSTALL_PREFIX=$(readlink -f ../../../../) \
#-DCMAKE_INCLUDE_DIRECTORIES_SYSTEM_BEFORE=ON -DCMAKE_INCLUDE_PATH="" -DCMAKE_SYSTEM_INCLUDE_PATH="" \


