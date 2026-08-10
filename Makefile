.DEFAULT_GOAL := help
.PHONY: flatpak flatpak-clean flatpak-sh snap snap-clean snapcraft-set-core18 linux version-header

ifeq ($(MINGW), 1)
$(info "CONFIG PKG_CONFIG MINGW64")
export PKG_CONFIG_LIBDIR := $(shell readlink -f build/mingw64/lib/pkgconfig)
export PKG_CONFIG_PATH := ""
else
$(info "CONFIG PKG_CONFIG ELSE")
endif

TESSERACT_LIBS=-ltesseract -llept
VERSION_FILE = src/Version.h
GIT_VERSION := $(shell git describe --tags --always --dirty)
OPTIONS=-g
CPP=g++
OPTIONS_LIB=-fPIC
BERGAMOT_INC=-Ibuild/linux/include/inference/src -Ibuild/linux/include/inference/marian-fork/src/ -Ibuild/linux/include/inference/marian-fork/src/3rd_party/ -Ibuild/linux/include/inference/ -Ibuild/linux/include/inference/3rd_party/ssplit-cpp/src/ssplit/ $(shell pkg-config --cflags openblas)
BERGAMOT_LIBS=-Lbuild/linux/lib -lmarian -lbergamot-translator-source $(shell pkg-config --libs openblas)
CURL_INC=$(shell pkg-config --cflags libcurl)
CURL_LIBS=$(shell pkg-config --libs libcurl)
PDFIUM_INC=-Ibuild/linux/include
PDFIUM_LIBS=-Lbuild/linux/lib -lpdfium
WX_SYSTEM := $(shell command -v wx-config 2>/dev/null)
ifeq ($(SAsan), 1)
$(info "-----------SAsan----------")
#ASAN_OPTIONS=detect_leaks=0 ./opolyglot #disable memory leak
OPTIONS= -g -fsanitize=address,undefined -fno-omit-frame-pointer -fsanitize-address-use-after-scope
endif
ifeq ($(SNAP), 1)
$(info "-----------SNAP----------")
CPP=g++-13
BERGAMOT_INC=-I$(SNAPCRAFT_STAGE)/bergamot/inference/src -I$(SNAPCRAFT_STAGE)/bergamot/inference/marian-fork/src/ -I$(SNAPCRAFT_STAGE)/bergamot/inference/marian-fork/src/3rd_party/ -I$(SNAPCRAFT_STAGE)/bergamot/inference/ -I$(SNAPCRAFT_STAGE)/bergamot/inference/3rd_party/ssplit-cpp/src/ssplit/
BERGAMOT_LIBS=-L$(SNAPCRAFT_STAGE)/usr/lib/$(CRAFT_ARCH_TRIPLET_BUILD_FOR) -lmarian -lbergamot-translator-source
PDFIUM_INC=-I$(SNAPCRAFT_STAGE)/pdfium/include
PDFIUM_LIBS=-L$(SNAPCRAFT_STAGE)/pdfium/lib -lpdfium
WX_CFLAGS=$(shell wx-config --cxxflags)
WX_LIBS=$(shell wx-config --libs base,core,xml,stc,html)
PORTAL_CFLAGS=$(shell pkg-config --cflags libportal,libportal-gtk3)
PORTAL_LIBS=$(shell pkg-config --libs libportal,libportal-gtk3)
OPTIONS = -D__SNAP
OPENSSL_LIBS=$(shell pkg-config --libs openssl)
else ifeq ($(FLATPAK), 1)
$(info "-----------FLATPAK----------")
TESSERACT_LIBS=-ltesseract -lleptonica
BERGAMOT_INCLUDE_SOURCE=./inference
BERGAMOT_INCLUDE_DEST=/app/include
BERGAMOT_INC=-I/app/include/inference/src -I/app/include/inference/marian-fork/src/ -I/app/include/inference/marian-fork/src/3rd_party/ -I/app/include/inference/ -I/app/include/inference/3rd_party/ssplit-cpp/src/ssplit/
BERGAMOT_LIBS=-L/app/lib -lmarian -lbergamot-translator-source
PORTAL_CFLAGS=$(shell pkg-config --cflags libportal,libportal-gtk3)
PORTAL_LIBS=$(shell pkg-config --libs libportal,libportal-gtk3)
WX_CFLAGS=$(shell wx-config --cxxflags base,core,xml,stc)
WX_LIBS=$(shell wx-config --libs base,core,xml,stc,html)
PDFIUM_INC=-I/app/include
PDFIUM_LIBS=-L/app/lib -lpdfium
OPTIONS = -g -D__FLATPAK
OPENSSL_LIBS=$(shell pkg-config --libs openssl)
else ifeq ($(MINGW),1)
$(info "-----------MINGW----------")
PDFIUM_INC=-Ibuild/mingw64/include
PDFIUM_LIBS=-Lbuild/mingw64/bin -lpdfium
OPTIONS=-mwindows
WX_CFLAGS=$(shell build/mingw64/bin/wx-config --prefix=build/mingw64 --cxxflags)
WX_LIBS=-Lbuild/mingw64/lib -lpthread -lwx_mswu-3.2-Windows
CPP=x86_64-w64-mingw32-g++
MINGW64_INC=-Ibuild/mingw64/include
CURL_INC=-Ibuild/mingw64/include
CURL_LIBS=-Lbuild/mingw64/lib -lcurl
TESSERACT_LIBS=-Lbuild/mingw64/lib -ltesseract -lleptonica
TESSERACT_CFLAGS=-Ibuild/mingw64/include
BERGAMOT_LIBS=-L./build/mingw64/lib -lmarian.dll -lbergamot-translator-source.dll
BERGAMOT_INC=-Ibuild/mingw64/include -Ibuild/mingw64/include/inference/src -Ibuild/mingw64/include/inference/marian-fork/src -Ibuild/mingw64/include/inference/marian-fork/src/3rd_party -Ibuild/mingw64/include/inference -Ibuild/mingw64/include/inference/3rd_party/ssplit-cpp/src/ssplit
PORTAL_CFLAGS =
PORTAL_LIBS =
OPENSSL_LIBS=-Lbuild/mingw64/lib64 -lcrypto
else ifeq ($(APPIMAGE), 1)
$(info "-----------APP IMAGE----------")
WX_CFLAGS=$(shell build/linux/bin/wx-config --prefix=$(shell pwd)/build/linux --cxxflags base,core,xml,stc,html)
WX_LIBS=$(shell build/linux/bin/wx-config --prefix=$(shell pwd)/build/linux --libs base,core,xml,stc,html)
PORTAL_CFLAGS=$(shell pkg-config --cflags libportal,libportal-gtk3)
PORTAL_LIBS=$(shell pkg-config --libs libportal,libportal-gtk3)
TESSERACT_CFLAGS=$(shell pkg-config --cflags tesseract,lept)
TESSERACT_LIBS =$(shell pkg-config --libs tesseract,lept)
OPTIONS=-D__APPIMAGE
OPENSSL_LIBS=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --libs openssl)
else
$(info "-----------else----------")
LIBPORTAL_EXISTS := $(shell pkg-config --exists libportal && echo yes || echo no)
TESSERACT_EXISTS := $(shell pkg-config --exists tesseract && echo yes || echo no)
CURL_EXISTS := $(shell pkg-config --exists libcurl && echo yes || echo no)
ifeq ($(WX_SYSTEM),)
	WX_CFLAGS=$(shell build/linux/bin/wx-config --prefix=$(shell pwd)/build/linux --cxxflags base,core,xml,stc,html)
	WX_LIBS=$(shell build/linux/bin/wx-config --prefix=$(shell pwd)/build/linux --libs base,core,xml,stc,html)
else
	WX_CFLAGS=$(shell wx-config --cxxflags base,core,xml,stc,html)
	WX_LIBS=$(shell wx-config  --libs base,core,xml,stc,html)
endif
ifeq ($(LIBPORTAL_EXISTS), no)
PORTAL_CFLAGS=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --define-prefix --cflags libportal,libportal-gtk3)
PORTAL_LIBS=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --define-prefix --libs libportal,libportal-gtk3)
else
PORTAL_CFLAGS=$(shell pkg-config --cflags libportal,libportal-gtk3)
PORTAL_LIBS=$(shell pkg-config --libs libportal,libportal-gtk3)
endif
ifeq ($(TESSERACT_EXISTS), no)
TESSERACT_CFLAGS=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --define-prefix --cflags lept,tesseract)
TESSERACT_LIBS=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --define-prefix --libs lept,tesseract)
else
TESSERACT_CFLAGS=$(shell pkg-config --cflags lept,tesseract)
TESSERACT_LIBS=$(shell pkg-config --libs lept,tesseract)
endif
ifeq ($(CURL_EXISTS), no)
CURL_INC=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --define-prefix --cflags libcurl)
CURL_LIBS=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --define-prefix --libs libcurl)
else
CURL_INC=$(shell pkg-config --cflags libcurl)
CURL_LIBS=$(shell pkg-config --libs libcurl)
endif
OPENSSL_LIBS=$(shell PKG_CONFIG_PATH=$(shell pwd)/build/linux/lib/pkgconfig pkg-config --libs openssl)
endif


bin:
	mkdir -p bin

all: help

help: 
	@echo "#--- CONFIGURE PKG_CONFIG_PATH ---"
	@echo 'export PKG_CONFIG_PATH="$$(readlink -f build/linux/lib/pkgconfig):$$PKG_CONFIG_PATH"'
	@echo "#---         COMPILE           ---"
	@echo "make MINGW=1 build"
	@echo "make FLATPAK=1 build"
	@echo "make SNAP=1 build"
	@echo "make compile-po"
	@echo "#--- CONFIGURE LD_LIBRARY_PATH ---"
	@echo 'export LD_LIBRARY_PATH=$$(readlink -f ./bin):$$LD_LIBRARY_PATH'
	@echo "#---       RUN SAsan           ---"
	@echo "make clean"
	@echo "make SAsan=1 build"
	@echo "cd bin"
	@echo 'ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:detect_stack_use_after_return=1" LSAN_OPTIONS="suppressions=../res/lsan_suppr.txt" ./opolyglot'

opolyglot:	build

clean: 
	rm -rf build/obj/*
	rm -rf bin/*


gettext:
	xgettext --package-name="OPolyglot" --package-version="$(GIT_VERSION)" --keyword="_" -kwxPLURAL:1,2 -kwxGETTEXT_IN_CONTEXT:1c,2 -kwxGETTEXT_IN_CONTEXT_PLURAL:1c,2,3 -kwxTRANSLATE -kwxTRANSLATE_IN_CONTEXT:1c,2 -kwxGetTranslation --from-code=utf-8 -D src -f src/ListTranslate.txt  --output src/locale/opolyglot.pot
	msgmerge -U src/locale/en/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/es/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/fr/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/cs/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/uk/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/sr/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/de/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/it/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/pl/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/ru/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/tr/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/pt/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/nl/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/da/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/ro/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/fi/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/sv/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/no/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/bg/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/el/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/az/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/sq/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/id/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/hu/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/sk/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/hr/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/sl/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/lt/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/lv/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/et/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/is/opolyglot.po src/locale/opolyglot.pot

compile-po:
	mkdir -p bin/locale/cs
	mkdir -p bin/locale/en
	mkdir -p bin/locale/es
	mkdir -p bin/locale/fr
	mkdir -p bin/locale/uk
	mkdir -p bin/locale/sr
	mkdir -p bin/locale/de
	mkdir -p bin/locale/it
	mkdir -p bin/locale/pl
	mkdir -p bin/locale/ru
	mkdir -p bin/locale/tr
	mkdir -p bin/locale/pt
	mkdir -p bin/locale/nl
	mkdir -p bin/locale/da
	mkdir -p bin/locale/ro
	mkdir -p bin/locale/fi
	mkdir -p bin/locale/sv
	mkdir -p bin/locale/no
	mkdir -p bin/locale/bg
	mkdir -p bin/locale/el
	mkdir -p bin/locale/az
	mkdir -p bin/locale/sq
	mkdir -p bin/locale/id
	mkdir -p bin/locale/hu
	mkdir -p bin/locale/sk
	mkdir -p bin/locale/hr
	mkdir -p bin/locale/sl
	mkdir -p bin/locale/lt
	mkdir -p bin/locale/lv
	mkdir -p bin/locale/et
	mkdir -p bin/locale/is
	msgfmt -vco bin/locale/cs/opolyglot.mo src/locale/cs/opolyglot.po
	msgfmt -vco bin/locale/en/opolyglot.mo src/locale/en/opolyglot.po
	msgfmt -vco bin/locale/es/opolyglot.mo src/locale/es/opolyglot.po
	msgfmt -vco bin/locale/fr/opolyglot.mo src/locale/fr/opolyglot.po
	msgfmt -vco bin/locale/uk/opolyglot.mo src/locale/uk/opolyglot.po
	msgfmt -vco bin/locale/sr/opolyglot.mo src/locale/sr/opolyglot.po
	msgfmt -vco bin/locale/de/opolyglot.mo src/locale/de/opolyglot.po
	msgfmt -vco bin/locale/it/opolyglot.mo src/locale/it/opolyglot.po
	msgfmt -vco bin/locale/pl/opolyglot.mo src/locale/pl/opolyglot.po
	msgfmt -vco bin/locale/ru/opolyglot.mo src/locale/ru/opolyglot.po
	msgfmt -vco bin/locale/tr/opolyglot.mo src/locale/tr/opolyglot.po
	msgfmt -vco bin/locale/pt/opolyglot.mo src/locale/pt/opolyglot.po
	msgfmt -vco bin/locale/nl/opolyglot.mo src/locale/nl/opolyglot.po
	msgfmt -vco bin/locale/da/opolyglot.mo src/locale/da/opolyglot.po
	msgfmt -vco bin/locale/ro/opolyglot.mo src/locale/ro/opolyglot.po
	msgfmt -vco bin/locale/fi/opolyglot.mo src/locale/fi/opolyglot.po
	msgfmt -vco bin/locale/sv/opolyglot.mo src/locale/sv/opolyglot.po
	msgfmt -vco bin/locale/no/opolyglot.mo src/locale/no/opolyglot.po
	msgfmt -vco bin/locale/bg/opolyglot.mo src/locale/bg/opolyglot.po
	msgfmt -vco bin/locale/el/opolyglot.mo src/locale/el/opolyglot.po
	msgfmt -vco bin/locale/az/opolyglot.mo src/locale/az/opolyglot.po
	msgfmt -vco bin/locale/sq/opolyglot.mo src/locale/sq/opolyglot.po
	msgfmt -vco bin/locale/id/opolyglot.mo src/locale/id/opolyglot.po
	msgfmt -vco bin/locale/hu/opolyglot.mo src/locale/hu/opolyglot.po
	msgfmt -vco bin/locale/sk/opolyglot.mo src/locale/sk/opolyglot.po
	msgfmt -vco bin/locale/hr/opolyglot.mo src/locale/hr/opolyglot.po
	msgfmt -vco bin/locale/sl/opolyglot.mo src/locale/sl/opolyglot.po
	msgfmt -vco bin/locale/lt/opolyglot.mo src/locale/lt/opolyglot.po
	msgfmt -vco bin/locale/lv/opolyglot.mo src/locale/lv/opolyglot.po
	msgfmt -vco bin/locale/et/opolyglot.mo src/locale/et/opolyglot.po
	msgfmt -vco bin/locale/is/opolyglot.mo src/locale/is/opolyglot.po

version-header:
	@echo "Generating $(VERSION_FILE)..."
	@echo "// This file is auto-generated. Do not edit." > $(VERSION_FILE)
	@echo "#ifndef VERSION_H" >> $(VERSION_FILE)
	@echo "#define VERSION_H" >> $(VERSION_FILE)
	@echo "/* PLEASE DO *NOT* EDIT THIS FILE! */" >> $(VERSION_FILE)
	@echo "#define OPOLYGLOT_VERSION \"$(GIT_VERSION)\"" >> $(VERSION_FILE)
	@echo "" >> $(VERSION_FILE)
	@echo "#endif // VERSION_H" >> $(VERSION_FILE)
	cat $(VERSION_FILE)

	
build: version-header bin build/obj build/obj/MainOPolyglot.o build/obj/GuiOPolyglot.o build/obj/OPolyglot.o build/obj/OPolyglotDownloadLanguage.o build/obj/OPolyglotSettings.o build/obj/Utils.o build/obj/OPolyglotFullscreenFrame.o build/obj/OPolyglotEvent.o build/obj/OPolyglotTaskBar.o build/obj/OPolyglotProcessingRules.o build/obj/OPolyglotAbout.o build/obj/LibOPolyglot.o build/obj/OPolyglotDocument.o build/obj/OPolyglotDebug.o build/obj/OPolyglotViewTextTranslate.o build/obj/OPolyglotEditTranslating.o linuxdeploy-plugin-gtk.sh linuxdeploy-x86_64.AppImage
	cp res/cacert.pem bin
ifdef MINGW
	@echo "USING MINGW"
	@if [ ! -f "bin/locale/en/opolyglot.mo" ]; then \
		echo "locale not found, run compile-po..."; \
		$(MAKE) compile-po; \
	fi
	x86_64-w64-mingw32-windres  -Ibuild/mingw64/include/wx-3.2 src/resource.rc -O coff -o build/obj/resource.res
endif
	$(CPP) build/obj/* $(PORTAL_LIBS) $(WX_LIBS)  $(OPTIONS) $(BERGAMOT_LIBS) $(OPENSSL_LIBS) $(TESSERACT_LIBS) $(CURL_LIBS) $(PDFIUM_LIBS) -o bin/opolyglot
ifeq ($(SNAP), 1)
	@echo "------SNAP------"
	
else ifeq ($(FLATPAK), 1)
	@echo "----FLATPAK----"
else ifeq ($(MINGW), 1)
	$(MAKE) MINGW=1 dll-copy
	cp doc/LICENSES.mingw64.txt bin/LICENSES.txt
	mkdir -p bin/res
	cp ./res/download.xml bin/res
	cp README.md bin
	strip --strip-debug bin/*.dll
else ifeq ($(APPIMAGE), 1)
	@echo "----AppImage----"
	mkdir -p bin/res

	cp res/download.xml bin/res
	cp doc/LICENSES.snap.txt bin/LICENSES.txt
	$(MAKE) compile-po
	chmod +x linuxdeploy-x86_64.AppImage
	chmod +x linuxdeploy-plugin-gtk.sh
	mkdir -p AppDir/res
	mkdir -p AppDir/usr/lib
	mkdir -p AppDir/usr/share/applications

	cp README.md AppDir/
	cp appimage/opolyglot.desktop AppDir/usr/share/applications
	cp bin/cacert.pem AppDir
	cp bin/res/download.xml AppDir/res
	cp bin/LICENSES.txt AppDir
	cp -r bin/locale AppDir
else
	@echo "----else----"
	cp build/linux/lib/libbergamot-translator-source.so bin
	cp build/linux/lib/libmarian.so bin
	cp build/linux/lib/libpdfium.so bin
	@if [ ! -f "bin/locale/en/opolyglot.mo" ]; then \
		echo "locale not found, run compile-po..."; \
		$(MAKE) compile-po; \
	fi
ifeq ($(LIBPORTAL_EXISTS), no)
	cp build/linux/lib/libportal-gtk3.so.1 bin
	cp build/linux/lib/libportal.so.1 bin
endif
ifeq ($(TESSERACT_EXISTS), no)
	cp build/linux/lib/libleptonica.so.6 bin
	cp build/linux/lib/libtesseract.so.5 bin
endif
ifeq ($(CURL_EXISTS), no)
	cp build/linux/lib/libcurl.so.4 bin
endif
	cp doc/LICENSES.snap.txt bin/LICENSES.txt
	cp README.md bin
	mkdir -p bin/res
	cp ./res/download.xml bin/res
endif
ifeq ($(MINGW),)
ifeq ($(WX_SYSTEM),)
	cp build/linux/lib/libwx_gtk3u-3.2.so.0 bin
endif
endif
	@echo "-----------------------FINISH-----------------------------"


linuxdeploy-plugin-gtk.sh:
ifeq ($(APPIMAGE), 1)
	wget https://github.com/linuxdeploy/linuxdeploy-plugin-gtk/raw/refs/heads/master/linuxdeploy-plugin-gtk.sh
endif

linuxdeploy-x86_64.AppImage:
ifeq ($(APPIMAGE), 1)
	wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
endif
	
	

build/obj/GuiOPolyglot.o: src/GuiOPolyglot.cpp src/GuiOPolyglot.cpp
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/GuiOPolyglot.cpp -o build/obj/GuiOPolyglot.o

build/obj/OPolyglotFullscreenFrame.o: src/OPolyglotFullscreenFrame.cpp src/OPolyglotFullscreenFrame.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotFullscreenFrame.cpp -o build/obj/OPolyglotFullscreenFrame.o

build/obj/OPolyglotAbout.o: src/OPolyglotAbout.cpp src/OPolyglotAbout.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS)   -c src/OPolyglotAbout.cpp -o build/obj/OPolyglotAbout.o

build/obj/OPolyglot.o: src/OPolyglot.cpp src/OPolyglot.h
	$(CPP) -Wall $(PORTAL_CFLAGS) $(WX_CFLAGS) $(PDFIUM_INC) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglot.cpp -o build/obj/OPolyglot.o


build/obj/OPolyglotDocument.o: src/OPolyglotDocument.cpp src/OPolyglotDocument.h
	$(CPP) -Wall $(WX_CFLAGS) $(PDFIUM_INC) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotDocument.cpp -o build/obj/OPolyglotDocument.o


build/obj/OPolyglotDebug.o: src/OPolyglotDebug.cpp src/OPolyglotDebug.h
	$(CPP) -Wall $(WX_CFLAGS) $(TESSERACT_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotDebug.cpp -o build/obj/OPolyglotDebug.o

build/obj/OPolyglotViewTextTranslate.o: src/OPolyglotViewTextTranslate.cpp src/OPolyglotViewTextTranslate.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotViewTextTranslate.cpp -o build/obj/OPolyglotViewTextTranslate.o


build/obj/OPolyglotEditTranslating.o: src/OPolyglotEditTranslating.cpp src/OPolyglotEditTranslating.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotEditTranslating.cpp -o build/obj/OPolyglotEditTranslating.o

build/obj/Utils.o: src/Utils.cpp src/Utils.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/Utils.cpp -o build/obj/Utils.o

build/obj/MainOPolyglot.o: src/MainOPolyglot.cpp src/MainOPolyglot.h src/Version.h
	@echo "$(WX_LIBS) $(WX_CFLAGS)"
	$(CPP) -Wall $(WX_CFLAGS) $(PDFIUM_INC) $(OPTIONS) $(DEBUG_OPTIONS) -c src/MainOPolyglot.cpp -o build/obj/MainOPolyglot.o

build/obj/OPolyglotSettings.o: src/OPolyglotSettings.cpp src/OPolyglotSettings.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotSettings.cpp -o build/obj/OPolyglotSettings.o

build/obj/OPolyglotDownloadLanguage.o: src/OPolyglotDownloadLanguage.cpp src/OPolyglotDownloadLanguage.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS)  $(CURL_INC) -c src/OPolyglotDownloadLanguage.cpp -o build/obj/OPolyglotDownloadLanguage.o


build/obj/OPolyglotEvent.o: src/OPolyglotEvent.cpp src/OPolyglotEvent.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotEvent.cpp -o build/obj/OPolyglotEvent.o


build/obj/OPolyglotTaskBar.o: src/OPolyglotTaskBar.cpp src/OPolyglotTaskBar.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotTaskBar.cpp -o build/obj/OPolyglotTaskBar.o

build/obj/OPolyglotProcessingRules.o: src/OPolyglotProcessingRules.cpp src/OPolyglotProcessingRules.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotProcessingRules.cpp -o build/obj/OPolyglotProcessingRules.o



build/obj/LibOPolyglot.o: src/LibOPolyglot.cpp 
	echo "$(TESSERACT_CFLAGS)"
	$(CPP) $(WX_CFLAGS) $(OPTIONS) $(OPTIONS_LIB) $(DEBUG_OPTIONS) $(TESSERACT_CFLAGS)  \
	-Wno-sign-compare -Wno-return-type -Wno-reorder -Wno-unused-value -Wno-deprecated-declarations \
	-Wno-template-id-cdtor -Wno-comment -Wno-unknown-pragmas -fPIC $(BERGAMOT_INC) \
	-c src/LibOPolyglot.cpp -o build/obj/LibOPolyglot.o


ifeq ($(MINGW), 1)


dll-system:  
	cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libstdc++-6.dll bin
	cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libgcc_s_seh-1.dll bin
	cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libgomp-1.dll bin
	cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll bin

bin/libmarian.dll: bin
	cp build/mingw64/bin/libmarian.dll bin

bin/libbergamot-translator-source.dll: bin
	cp build/mingw64/bin/libbergamot-translator-source.dll bin

bin/libcrypto-3-x64.dll: bin
	cp build/mingw64/bin/libcrypto-3-x64.dll bin

bin/libcurl.dll: bin
	cp build/mingw64/bin/libcurl.dll bin

bin/libopenblas.dll: bin
	cp build/mingw64/bin/libopenblas.dll bin

bin/libpcre2-8-0.dll: bin
	cp build/mingw64/bin/libpcre2-8-0.dll bin

bin/libtesseract-5.dll: bin
	cp build/mingw64/bin/libtesseract-5.dll bin

bin/wxmsw32u_gcc_custom.dll: bin
	cp build/mingw64/bin/wxmsw32u_gcc_custom.dll bin

bin/libleptonica-1.87.0.dll: bin
	cp build/mingw64/bin/libleptonica-1.87.0.dll bin

bin/libpng16.dll: bin
	cp build/mingw64/bin/libpng16.dll bin

bin/libssl-3-x64.dll: bin
	cp build/mingw64/bin/libssl-3-x64.dll bin

bin/libtiff-6.dll: bin
	cp build/mingw64/bin/libtiff-6.dll bin

bin/libz.dll: bin
	cp build/mingw64/bin/libz.dll bin


bin/pdfium.dll: bin
	cp build/mingw64/bin/pdfium.dll bin

dll-copy: bin/libbergamot-translator-source.dll bin/libmarian.dll  bin/libcrypto-3-x64.dll bin/libcurl.dll bin/libopenblas.dll bin/libpcre2-8-0.dll bin/libtesseract-5.dll bin/wxmsw32u_gcc_custom.dll dll-system bin/libleptonica-1.87.0.dll  bin/libssl-3-x64.dll bin/libz.dll bin/pdfium.dll
	
endif

RUNTIME = org.freedesktop.Platform
VERSION = 25.08
RUNTIME_FULL_ID = $(RUNTIME)//$(VERSION)
SDK = org.freedesktop.Sdk
SDK_FULL_ID = $(SDK)//$(VERSION)

flatpak-check-env:
	@echo "Checking the environment for $(RUNTIME_FULL_ID) , $(SDK_FULL_ID) , $(SDK_VALA_FULL_ID)"
	@flatpak info $(RUNTIME_FULL_ID) > /dev/null 2>&1 || $(MAKE) flatpak-install-runtime
	@flatpak info $(SDK_FULL_ID) > /dev/null 2>&1 || $(MAKE) flatpak-install-sdk
	@flatpak info org.flatpak.Builder > /dev/null 2>&1 || $(MAKE) flatpak-install-builder

flatpak-install-runtime:
	@echo "Package $(RUNTIME_FULL_ID) not found. Installation..."
	flatpak install --user -y flathub $(RUNTIME_FULL_ID)

flatpak-install-sdk:
	@echo "Package $(SDK_FULL_ID) not found. Installation..."
	flatpak install --user -y flathub $(SDK_FULL_ID)


flatpak-install-builder:
	flatpak install --user org.flatpak.Builder


flatpak-clean:
	rm -rf build/flatpak

flatpak: flatpak-check-env
	ls ./
	$(MAKE) -f Makefile flatpak-check-env
	mkdir -p build/flatpak
	mkdir -p build/flatpakrepo
	flatpak-builder --repo=build/flatpakrepo --force-clean build/flatpak flatpak/io.sourceforge.opolyglot.yaml
	flatpak build-bundle build/flatpakrepo opolyglot-x86_64.flatpak io.sourceforge.opolyglot --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
	flatpak build-bundle build/flatpakrepo opolyglot-x86_64-debug.flatpak runtime/io.sourceforge.opolyglot.Debug/x86_64/master --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
	
#flatpak run org.flatpak.Builder --repo=build/flatpakrepo --force-clean build/flatpak flatpak/io.sourceforge.opolyglot.yaml

flatpak-sh:
	flatpak-builder --user --repo=build/flatpakrepo --force-clean build/flatpak flatpak/io.sourceforge.opolyglot.yaml
# flatpak-builder --run build/flatpak/build flatpak/io.sourceforge.opolyglot.yaml sh

snap-delete-snapcraft:
	snap remove --purge snapcraft

snapcraft-set-core18:
	command -v snapcraft > /dev/null 2>&1 && $(MAKE) snap-delete-snapcraft
	@test -f snap/snapcraft.yaml && echo "file in place"

snap-clean:
	snapcraft clean --use-lxd --verbose

snap:
	snapcraft pack --use-lxd --debug --verbose


run: 
	export LD_LIBRARY_PATH=$$(readlink -f ./bin):$$LD_LIBRARY_PATH && cd bin && ./opolyglot

linux:
	$(MAKE) build
	$(MAKE)	compile-po 

build/obj:
	mkdir -p build/obj

%:
	$(MAKE) help
