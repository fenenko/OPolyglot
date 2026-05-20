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
cmake  -DSSPLIT_USE_INTERNAL_PCRE2=ON -DCMAKE_BUILD_TYPE=Release ../
echo "Build translations $(date)"
make -j$(nproc)
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
cd ..
