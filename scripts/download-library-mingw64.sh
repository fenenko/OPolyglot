#!/bin/sh
mkdir -p ../build/mingw64
cd ../build
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-leptonica-1.87.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-leptonica-1.87.0-1-any.pkg.tar.zst
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-tesseract-ocr-5.5.2-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-tesseract-ocr-5.5.2-1-any.pkg.tar.zst
cp mingw64/bin/libtesseract-5.5.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libtomcrypt-1.18.2-4-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libtomcrypt-1.18.2-4-any.pkg.tar.zst
cp mingw64/bin/libtomcrypt-1.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libarchive-3.8.5-2-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libarchive-3.8.5-2-any.pkg.tar.zst
cp mingw64/bin/libarchive-13.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libb2-0.98.1-3-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libb2-0.98.1-3-any.pkg.tar.zst 
cp mingw64/bin/libb2-1.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-bzip2-1.0.8-3-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-bzip2-1.0.8-3-any.pkg.tar.zst 
cp mingw64/bin/libbz2-1.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-openssl-3.6.1-3-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-openssl-3.6.1-3-any.pkg.tar.zst
cp mingw64/bin/libcrypto-3-x64.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-expat-2.7.4-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-expat-2.7.4-1-any.pkg.tar.zst 
cp mingw64/bin/libexpat-1.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libiconv-1.18-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libiconv-1.18-1-any.pkg.tar.zst 
cp mingw64/bin/libiconv-2.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-lz4-1.10.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-lz4-1.10.0-1-any.pkg.tar.zst
cp mingw64/bin/liblz4.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-xz-5.8.2-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-xz-5.8.2-1-any.pkg.tar.zst
cp mingw64/bin/liblzma-5.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-zstd-1.5.7-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-zstd-1.5.7-1-any.pkg.tar.zst
cp mingw64/bin/libzstd.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-curl-8.18.0-4-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-curl-8.18.0-4-any.pkg.tar.zst 
cp mingw64/bin/libcurl-4.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-giflib-5.2.2-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-giflib-5.2.2-1-any.pkg.tar.zst
cp mingw64/bin/libgif-7.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-brotli-1.2.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-brotli-1.2.0-1-any.pkg.tar.zst 
cp mingw64/bin/libbrotlidec.dll ../bin
cp mingw64/bin/libbrotlicommon.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libidn2-2.3.8-4-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libidn2-2.3.8-4-any.pkg.tar.zst
cp mingw64/bin/libidn2-0.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-gettext-runtime-1.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-gettext-runtime-1.0-1-any.pkg.tar.zst
cp mingw64/bin/libintl-8.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libunistring-1.3-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libunistring-1.3-1-any.pkg.tar.zst 
cp mingw64/bin/libunistring-5.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-nghttp2-1.68.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-nghttp2-1.68.0-1-any.pkg.tar.zst 
cp mingw64/bin/libnghttp2-14.dll ../bin
cp mingw64/bin/libssl-3-x64.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-nghttp3-1.15.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-nghttp3-1.15.0-1-any.pkg.tar.zst
cp mingw64/bin/libnghttp3-9.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-ngtcp2-1.21.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-ngtcp2-1.21.0-1-any.pkg.tar.zst
cp mingw64/bin/libngtcp2-16.dll ../bin
cp mingw64/bin/libngtcp2_crypto_ossl-0.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libpsl-0.21.5-3-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libpsl-0.21.5-3-any.pkg.tar.zst
cp mingw64/bin/libpsl-5.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libssh2-1.11.1-2-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libssh2-1.11.1-2-any.pkg.tar.zst 
cp mingw64/bin/libssh2-1.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libjpeg-turbo-3.1.3-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libjpeg-turbo-3.1.3-1-any.pkg.tar.zst 
cp mingw64/bin/libjpeg-8.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-openjpeg2-2.5.4-2-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-openjpeg2-2.5.4-2-any.pkg.tar.zst 
cp mingw64/bin/libopenjp2-7.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libpng-1.6.55-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libpng-1.6.55-1-any.pkg.tar.zst
cp mingw64/bin/libpng16-16.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libtiff-4.7.1-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libtiff-4.7.1-1-any.pkg.tar.zst 
cp mingw64/bin/libtiff-6.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libwebp-1.6.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libwebp-1.6.0-1-any.pkg.tar.zst 
cp mingw64/bin/libwebp-7.dll ../bin
cp mingw64/bin/libwebpmux-3.dll ../bin
cp mingw64/bin/libsharpyuv-0.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libdeflate-1.25-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-libdeflate-1.25-1-any.pkg.tar.zst
cp mingw64/bin/libdeflate.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-jbigkit-2.1-5-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-jbigkit-2.1-5-any.pkg.tar.zst
cp mingw64/bin/libjbig-0.dll ../bin
wget https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-lerc-4.0.0-1-any.pkg.tar.zst
tar -xvf mingw-w64-x86_64-lerc-4.0.0-1-any.pkg.tar.zst
cp mingw64/bin/libLerc.dll ../bin
