OPTIONS=-g 
#-fsanitize=undefined -fsanitize=unreachable  -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract  #-fsanitize=thread 
CPP=g++
#CPP=clang++
WX_CFLAGS=$(shell wx-config --cxxflags)
WX_LIBS=$(shell wx-config --libs)
BERGAMOT_INC=-Ibuild/include/inference/src -Ibuild/include/inference/marian-fork/src/ -Ibuild/include/inference/marian-fork/src/3rd_party/ -Ibuild/include/inference/ -Ibuild/include/inference/3rd_party/ssplit-cpp/src/ssplit/
#BERGAMOT_INC=-Ibuild/include -Ibuild/include/half_float -Ibuild/include/marian-fork/src/3rd_party/ -Ibuild/include/marian-fork/src -Ibuild/include/3rd_party/ssplit-cpp/src/ssplit/ -Ibuild/include/src/translator

ifdef WIN32
	
WX_CFLAGS=-Ibin/win32/lib/gcc1420_x64_dll/mswu -Ibin/win32/include 
CPP=x86_64-w64-mingw32-g++
TOMCRYPT=-L./build/mingw64/lib -ltomcrypt
MINGW64_INC=-Ibuild/mingw64/include
WX_LIBS=-Lbin/win32/lib/gcc1420_x64_dll/ -lwxbase32u -lwxbase32u_net -lwxmsw32u_core -lwxmsw32u_adv -lwxmsw32u_html -lwxmsw32u_webview
BERGAMOT_INC=-Ibuild/src/bergamot-translator/src/ -Ibuild/src/bergamot-translator/3rd_party/marian-dev/src -Ibuild/src/bergamot-translator/3rd_party/marian-dev/src/3rd_party/ -Ibuild/src/bergamot-translator -Ibuild/src/bergamot-translator/3rd_party/ssplit-cpp/src/ssplit/
TESSERACT_LIBS=-L./build/mingw64/lib -ltesseract 
BERGAMOT_LIBS=-L./bin/win32 -lmarian -lbergamot-translator
else
TESSERACT_LIBS=-ltesseract 
TOMCRYPT=-ltomcrypt
BERGAMOT_LIBS=-L./bin -lmarian -lbergamot-translator-source
endif
BERGAMOT_INCLUDE_DEST=build/include
BERGAMOT_INCLUDE_SOURCE=build/src/translations/inference
ifdef FLATPAK
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


#TRANSLATOR_LIB=-Lbuild/ -ltranslator

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
	export LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH

backup:
	git bundle create ../BackupOPolyglot.bundle --all

clean: 
	rm -r build/obj
	rm OPolyglot

translator:
	xgettext --keyword="_" -kwxPLURAL:1,2 -kwxGETTEXT_IN_CONTEXT:1c,2 -kwxGETTEXT_IN_CONTEXT_PLURAL:1c,2,3 -kwxTRANSLATE -kwxTRANSLATE_IN_CONTEXT:1c,2 -kwxGetTranslation --from-code=utf-8 -D src -f src/ListTranslate.txt  --output opolyglot.pot
	msgmerge -U locale/en/opolyglot.po opolyglot.pot
	msgmerge -U locale/es/opolyglot.po opolyglot.pot
	msgmerge -U locale/fr/opolyglot.po opolyglot.pot
	msgmerge -U locale/cs/opolyglot.po opolyglot.pot
	msgmerge -U locale/ua/opolyglot.po opolyglot.pot
#mkdir -p locale
#mkdir -p locale/uk
	
#cp opolyglot.pot locale/uk
#msginit -l uk -o locale/uk/opolyglot.po
	
	
build: build/obj build/obj/MainOPolyglot.o build/obj/GuiOPolyglot.o build/obj/OPolyglot.o build/obj/OPolyglotDownloadLanguage.o build/obj/OPolyglotSetup.o build/obj/Utils.o build/obj/OPolyglotFullscreenFrame.o build/obj/OPolyglotThread.o build/obj/OPolyglotEvent.o build/obj/OPolyglotType.o  build/obj/OPolyglotTaskBar.o build/obj/OPolyglotProcessingRules.o build/obj/OPolyglotAbout.o
	#git push ../BackupOPolyglot/OPolyglot
	$(CPP) -Wall -std=c++11 -pthread -Wl,--no-as-needed -fPIC -Wno-unused-result \
	$(WX_LIBS)  $(OPTIONS) $(DEBUG_OPTIONS) -std=c++17 -Wextra -lstdc++ $(TOMCRYPT)  build/obj/* \
	-o OPolyglot

build/obj/GuiOPolyglot.o: src/GuiOPolyglot.cpp src/GuiOPolyglot.cpp
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/GuiOPolyglot.cpp -o build/obj/GuiOPolyglot.o

build/obj/OPolyglotFullscreenFrame.o: src/OPolyglotFullscreenFrame.cpp src/OPolyglotFullscreenFrame.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotFullscreenFrame.cpp -o build/obj/OPolyglotFullscreenFrame.o

build/obj/OPolyglotThread.o: src/OPolyglotThread.cpp src/OPolyglotThread.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotThread.cpp -o build/obj/OPolyglotThread.o
	
build/obj/OPolyglotAbout.o: src/OPolyglotAbout.cpp src/OPolyglotAbout.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotAbout.cpp -o build/obj/OPolyglotAbout.o

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
	$(CPP) -Wall -fPIC $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotType.cpp -o build/obj/OPolyglotType.o


build/obj/OPolyglotTaskBar.o: src/OPolyglotTaskBar.cpp src/OPolyglotTaskBar.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotTaskBar.cpp -o build/obj/OPolyglotTaskBar.o

build/obj/OPolyglotProcessingRules.o: src/OPolyglotProcessingRules.cpp src/OPolyglotProcessingRules.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotProcessingRules.cpp -o build/obj/OPolyglotProcessingRules.o



build/obj/OPolyglotDynamic.o: src/OPolyglotDynamic.cpp 
	$(CPP) -Wall -fPIC $(WX_CFLAGS) $(MINGW64_INC) $(OPTIONS) $(DEBUG_OPTIONS) \
	$(BERGAMOT_INC) \
	-Wno-sign-compare -Wno-return-type -Wno-reorder -Wno-unused-value -Wno-deprecated-declarations \
	-Wno-template-id-cdtor -Wno-comment -Wno-unknown-pragmas \
	-fPIC -c src/OPolyglotDynamic.cpp -o build/obj/OPolyglotDynamic.o

libtranslator: include build/obj build/obj/OPolyglotDynamic.o build/obj/OPolyglotType.o
	$(CPP) $(DEBUG_OPTIONS) $(MINGW64_INC) -shared -Wall -std=c++11 -pthread  -Wl,--error-unresolved-symbols -Wl,--fatal-warnings -Wl,--no-as-needed -fPIC $(OPTIONS) $(WX_LIBS) $(TESSERACT_LIBS) $(BERGAMOT_LIBS) build/obj/OPolyglotDynamic.o build/obj/OPolyglotType.o -o build/libopolyglot-ocr-translator.so 
	rm build/obj/OPolyglotDynamic.o
	cp build/libopolyglot-ocr-translator.so bin


include:
ifndef FLATPAK
	mkdir -p $(BERGAMOT_INCLUDE_DEST)
	cp -r $(BERGAMOT_INCLUDE_SOURCE) $(BERGAMOT_INCLUDE_DEST)
	echo $(BERGAMOT_INCLUDE_SOURCE)
	echo $(BERGAMOT_INCLUDE_DEST)
endif


build/obj:
	mkdir -p build/obj
