#!/bin/sh

mkdir -p ../build
cd ../build
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-wxwidgets3.2-msw-3.2.9-1-any.pkg.tar.zst
tar -vxf mingw-w64-x86_64-wxwidgets3.2-msw-3.2.9-1-any.pkg.tar.zst
