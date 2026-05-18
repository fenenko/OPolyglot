#!/bin/sh

mkdir -p ../build/src
mkdir -p ../build/linux/bin
mkdir -p ../build/linux/lib
mkdir -p ../build/linux/include
cd ../build/src

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
cmake  -DSSPLIT_USE_INTERNAL_PCRE2=ON -DUSE_RUY=ON -DUSE_RUY_SGEMM=ON -DCMAKE_BUILD_TYPE=Release ../
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
