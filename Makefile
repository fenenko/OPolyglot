OPTIONS=-g 
#-fsanitize=undefined -fsanitize=unreachable  -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract  #-fsanitize=thread 
CPP=g++
#CPP=clang++
WX_CFLAGS=$(shell wx-config --cxxflags)
WX_LIBS=$(shell wx-config --libs)
BERGAMOT_INC=-Ibuild/linux/include/inference/src -Ibuild/linux/include/inference/marian-fork/src/ -Ibuild/linux/include/inference/marian-fork/src/3rd_party/ -Ibuild/linux/include/inference/ -Ibuild/linux/include/inference/3rd_party/ssplit-cpp/src/ssplit/
#BERGAMOT_INC=-Ibuild/include -Ibuild/include/half_float -Ibuild/include/marian-fork/src/3rd_party/ -Ibuild/include/marian-fork/src -Ibuild/include/3rd_party/ssplit-cpp/src/ssplit/ -Ibuild/include/src/translator
BERGAMOT_LIBS=-Lbuild/linux/bin -lmarian -lbergamot-translator-source
OUTPUT_LIB=libopolyglot-ocr-translator.so
ifdef WIN32
$(echo "WIN32")
WX_CFLAGS=$(shell build/mingw64/bin/wx-config --cxxflags)
WX_LIBS=$(shell build/mingw64/bin/wx-config --libs all --cxxflags)
TOMCRYPT_INC=-Ibuild/mingw64/include
CPP=x86_64-w64-mingw32-g++
TOMCRYPT=-L./build/mingw64/lib -ltomcrypt
MINGW64_INC=-Ibuild/mingw64/include
BERGAMOT_INC=-Ibuild/src/bergamot-translator/src/ -Ibuild/src/bergamot-translator/3rd_party/marian-dev/src -Ibuild/src/bergamot-translator/3rd_party/marian-dev/src/3rd_party/ -Ibuild/src/bergamot-translator -Ibuild/src/bergamot-translator/3rd_party/ssplit-cpp/src/ssplit/
OUTPUT_LIB=libopolyglot-ocr-translator.dll
TESSERACT_LIBS=-L./build/mingw64/lib -ltesseract
BERGAMOT_LIBS=-L./build/mingw64/lib -lmarian.dll -lbergamot-translator-source.dll
BERGAMOT_INC=-Ibuild/mingw64/include/inference/src -Ibuild/mingw64/include/inference/marian-fork/src -Ibuild/mingw64/include/inference/marian-fork/src/3rd_party -Ibuild/mingw64/include/inference -Ibuild/mingw64/include/inference/3rd_party/ssplit-cpp/src/ssplit
OPTIONS_LIB=
else
OPTIONS_LIB=-fPIC
TESSERACT_LIBS=-ltesseract 
TOMCRYPT=-ltomcrypt
endif
BERGAMOT_INCLUDE_DEST=build/include
BERGAMOT_INCLUDE_SOURCE=build/src/translations/inference
ifdef FLATPAK
OPTIONS=
BERGAMOT_INCLUDE_SOURCE=./inference
BERGAMOT_INCLUDE_DEST=/app/include
BERGAMOT_INC=-I/app/include/inference/src -I/app/include/inference/marian-fork/src/ -I/app/include/inference/marian-fork/src/3rd_party/ -I/app/include/inference/ -I/app/include/inference/3rd_party/ssplit-cpp/src/ssplit/
OPTIONS += -D__FLATPAK
endif
#$(shell pkg-config --libs valgrind)
#BERGAMOTH_PATH=/home/oleksandr/tmp/build/bergamot-translator/src/
#BERGAMOTG_ROOT_PATH=/home/oleksandr/tmp/build/bergamot-translator
MARIAN_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/marian-dev/src/

MARIAN_DEP_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/marian-dev/src/3rd_party/
SPLIT_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/ssplit-cpp/src/ssplit/
VERSION_HDR := src/OPolyglotVersion.h
OPOLYGLOT_VERSION_NAME := $(shell sed -n 's/^[[:space:]]*#define[[:space:]]\+OPOLYGLOT_VERSION_NAME[[:space:]]\+\(.*\)/\1/p' $(VERSION_HDR) | sed -e 's/^[[:space:]]*"\(.*\)"[[:space:]]*$$/\1/')
OPOLYGLOT_VERSION_MINOR := $(shell sed -n 's/^[[:space:]]*#define[[:space:]]\+OPOLYGLOT_VERSION_MINOR[[:space:]]\+\(.*\)/\1/p' $(VERSION_HDR) | sed -e 's/^[[:space:]]*"\(.*\)"[[:space:]]*$$/\1/')


#TRANSLATOR_LIB=-Lbuild/ -ltranslator

bin:
	mkdir -p bin

all:
	echo "make sanitize-mem"
	echo "make valgrind-mem"

help: all
	echo "make WIN32=1 build"

win32: WX_CFLAGS=-Ibin/win32/lib/gcc1420_x64_dll/mswu -Ibin/win32/include -DSTRICT -DHAVE_W32API_H -D__WXMSW__ -D__WINDOWS__ CPP=x86_64-w64-mingw32-g++ build
win32: build

sanitize-mem: OPTIONS += -fsanitize=undefined -fsanitize=unreachable  -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract
sanitize-mem: clean build run


valgrind-mem: clean build
	valgrind --tool=memcheck ./OPolyglot

test1:
	echo "test"

test2: test1

run: OPolyglot
	LSAN_OPTIONS="suppressions=./LASan.supp" ./OPolyglot

OPolyglot:	build


config-library:
	export LD_LIBRARY_PATH=build/linux/bin:$LD_LIBRARY_PATH

backup:
	git bundle create ../BackupOPolyglot.bundle --all

clean: 
	rm -rf build/obj/*
	rm -rf bin/*

translator:
	xgettext --package-name="OPolyglot" --package-version="$(OPOLYGLOT_VERSION_NAME) $(OPOLYGLOT_VERSION_MINOR)" --keyword="_" -kwxPLURAL:1,2 -kwxGETTEXT_IN_CONTEXT:1c,2 -kwxGETTEXT_IN_CONTEXT_PLURAL:1c,2,3 -kwxTRANSLATE -kwxTRANSLATE_IN_CONTEXT:1c,2 -kwxGetTranslation --from-code=utf-8 -D src -f src/ListTranslate.txt  --output src/locale/opolyglot.pot
	msgmerge -U src/locale/en/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/es/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/fr/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/cs/opolyglot.po src/locale/opolyglot.pot
	msgmerge -U src/locale/uk/opolyglot.po src/locale/opolyglot.pot
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

translatormo:
	mkdir -p bin/locale/cs
	mkdir -p bin/locale/en
	mkdir -p bin/locale/es
	mkdir -p bin/locale/fr
	mkdir -p bin/locale/uk
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
	msgfmt -vco bin/locale/cs/opolyglot.mo src/locale/cs/opolyglot.po
	msgfmt -vco bin/locale/en/opolyglot.mo src/locale/en/opolyglot.po
	msgfmt -vco bin/locale/es/opolyglot.mo src/locale/es/opolyglot.po
	msgfmt -vco bin/locale/fr/opolyglot.mo src/locale/fr/opolyglot.po
	msgfmt -vco bin/locale/uk/opolyglot.mo src/locale/uk/opolyglot.po
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


#mkdir -p locale
#mkdir -p locale/uk
	
#cp opolyglot.pot locale/uk
#msginit -l uk -o locale/uk/opolyglot.po -i opolyglot.pot
	
	
build: bin build/obj build/obj/MainOPolyglot.o build/obj/GuiOPolyglot.o build/obj/OPolyglot.o build/obj/OPolyglotDownloadLanguage.o build/obj/OPolyglotSetup.o build/obj/Utils.o build/obj/OPolyglotFullscreenFrame.o build/obj/OPolyglotThread.o build/obj/OPolyglotEvent.o build/obj/OPolyglotType.o  build/obj/OPolyglotTaskBar.o build/obj/OPolyglotProcessingRules.o build/obj/OPolyglotAbout.o translatormo
	$(CPP) build/obj/* $(WX_LIBS) $(TOMCRYPT) $(OPTIONS) -o bin/OPolyglot
ifndef FLATPAK
	@echo "NOT USING FLATPAK"
	mkdir -p bin/res
	cp ./res/download.xml bin/res
endif

ifdef WIN32
bin/zlib1.dll: bin
	cp /usr/x86_64-w64-mingw32/lib/zlib1.dll bin
bin/libgcc_s_seh-1.dll: bin
	cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libgcc_s_seh-1.dll bin
	cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libstdc++-6.dll bin
bin/wxmsw32u_gcc_custom.dll: bin
	cp build/mingw64/bin/wxmsw32u_gcc_custom.dll bin
bin/libgomp-1.dll: bin
	cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libgomp-1.dll bin
bin/libwinpthread-1.dll: bin
	cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll bin
bin/libtommath.dll: bin
	cp build/mingw64/bin/libtommath.dll bin
bin/libtomcrypt.dll: bin
	cp build/mingw64/bin/libtomcrypt.dll bin
bin/libbergamot-translator-source.dll: bin
	cp build/mingw64/bin/libbergamot-translator-source.dll bin
bin/libmarian.dll: bin
	cp build/mingw64/bin/libmarian.dll bin
bin/libtesseract-5.dll: bin
	cp build/mingw64/bin/libtesseract-5.dll bin
bin/libopenblas.dll: bin
	cp build/mingw64/bin/libopenblas.dll bin
bin/libpcre2-8-0.dll: bin
	cp build/mingw64/bin/libpcre2-8-0.dll bin
bin/libleptonica-1.88.0.dll: bin
	cp build/mingw64/bin/libleptonica-1.88.0.dll bin
build: bin/zlib1.dll bin/libgcc_s_seh-1.dll bin/wxmsw32u_gcc_custom.dll bin/libgomp-1.dll bin/libwinpthread-1.dll bin/libtommath.dll bin/libtomcrypt.dll bin/libbergamot-translator-source.dll bin/libmarian.dll bin/libtesseract-5.dll bin/libopenblas.dll bin/libpcre2-8-0.dll bin/libleptonica-1.88.0.dll

endif

build/obj/GuiOPolyglot.o: src/GuiOPolyglot.cpp src/GuiOPolyglot.cpp
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/GuiOPolyglot.cpp -o build/obj/GuiOPolyglot.o

build/obj/OPolyglotFullscreenFrame.o: src/OPolyglotFullscreenFrame.cpp src/OPolyglotFullscreenFrame.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotFullscreenFrame.cpp -o build/obj/OPolyglotFullscreenFrame.o

build/obj/OPolyglotThread.o: src/OPolyglotThread.cpp src/OPolyglotThread.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotThread.cpp -o build/obj/OPolyglotThread.o
	
build/obj/OPolyglotAbout.o: src/OPolyglotAbout.cpp src/OPolyglotAbout.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) $(TOMCRYPT_INC)  -c src/OPolyglotAbout.cpp -o build/obj/OPolyglotAbout.o

build/obj/OPolyglot.o: src/OPolyglot.cpp src/OPolyglot.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglot.cpp -o build/obj/OPolyglot.o

build/obj/Utils.o: src/Utils.cpp src/Utils.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/Utils.cpp -o build/obj/Utils.o

build/obj/MainOPolyglot.o: src/MainOPolyglot.cpp src/MainOPolyglot.h src/Version.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/MainOPolyglot.cpp -o build/obj/MainOPolyglot.o

build/obj/OPolyglotSetup.o: src/OPolyglotSetup.cpp src/OPolyglotSetup.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotSetup.cpp -o build/obj/OPolyglotSetup.o

build/obj/OPolyglotDownloadLanguage.o: src/OPolyglotDownloadLanguage.cpp src/OPolyglotDownloadLanguage.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) $(TOMCRYPT_INC) -c src/OPolyglotDownloadLanguage.cpp -o build/obj/OPolyglotDownloadLanguage.o


build/obj/OPolyglotEvent.o: src/OPolyglotEvent.cpp src/OPolyglotEvent.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotEvent.cpp -o build/obj/OPolyglotEvent.o


build/obj/OPolyglotType.o: src/OPolyglotType.cpp src/OPolyglotType.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) $(OPTIONS_LIB) -c src/OPolyglotType.cpp -o build/obj/OPolyglotType.o


build/obj/OPolyglotTaskBar.o: src/OPolyglotTaskBar.cpp src/OPolyglotTaskBar.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotTaskBar.cpp -o build/obj/OPolyglotTaskBar.o

build/obj/OPolyglotProcessingRules.o: src/OPolyglotProcessingRules.cpp src/OPolyglotProcessingRules.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotProcessingRules.cpp -o build/obj/OPolyglotProcessingRules.o



build/obj/OPolyglotDynamic.o: src/OPolyglotDynamic.cpp 
	$(CPP) $(WX_CFLAGS) $(MINGW64_INC) $(OPTIONS) $(OPTIONS_LIB) $(DEBUG_OPTIONS)  \
	-Wno-sign-compare -Wno-return-type -Wno-reorder -Wno-unused-value -Wno-deprecated-declarations \
	-Wno-template-id-cdtor -Wno-comment -Wno-unknown-pragmas -fPIC $(BERGAMOT_INC) \
	-c src/OPolyglotDynamic.cpp -o build/obj/OPolyglotDynamic.o

libtranslator: include build/obj build/obj/OPolyglotDynamic.o build/obj/OPolyglotType.o
	$(CPP) $(MINGW64_INC)  $(OPTIONS) $(OPTIONS_LIB)  -shared   -o bin/$(OUTPUT_LIB) build/obj/OPolyglotDynamic.o build/obj/OPolyglotType.o $(WX_LIBS) $(BERGAMOT_LIBS) $(TESSERACT_LIBS)
	rm build/obj/OPolyglotDynamic.o


include:
ifndef FLATPAK
	mkdir -p $(BERGAMOT_INCLUDE_DEST)
	cp -r $(BERGAMOT_INCLUDE_SOURCE) $(BERGAMOT_INCLUDE_DEST)
	echo $(BERGAMOT_INCLUDE_SOURCE)
	echo $(BERGAMOT_INCLUDE_DEST)
endif


build/obj:
	mkdir -p build/obj
