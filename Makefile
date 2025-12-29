OPTIONS=-g #-fsanitize=undefined -fsanitize=unreachable  -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract  #-fsanitize=thread 
CPP=g++
#CPP=clang++
WX_CFLAGS=$(shell wx-config --cxxflags)
WX_LIBS=$(shell wx-config --libs)
#$(shell pkg-config --libs valgrind)
BERGAMOTH_PATH=/home/oleksandr/tmp/build/bergamot-translator/src/
BERGAMOTG_ROOT_PATH=/home/oleksandr/tmp/build/bergamot-translator
MARIAN_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/marian-dev/src/

MARIAN_DEP_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/marian-dev/src/3rd_party/
SPLIT_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/ssplit-cpp/src/ssplit/

BERGAMOT_LIBS=-L./bin -lmarian -lbergamot-translator-source

TESSERACT_LIBS=-ltesseract 
#TRANSLATOR_LIB=-Lbuild/ -ltranslator

all:
	echo "make sanitize-mem"
	echo "make valgrind-mem"

help: all

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
	
build: build/obj build/obj/MainOPolyglot.o build/obj/GuiOPolyglot.o build/obj/OPolyglot.o build/obj/OPolyglotDownloadLanguage.o build/obj/OPolyglotSetup.o build/obj/Utils.o build/obj/OPolyglotFullscreenFrame.o build/obj/OPolyglotThread.o build/obj/OPolyglotEvent.o build/obj/OPolyglotType.o  build/obj/OPolyglotTaskBar.o build/obj/OPolyglotProcessingRules.o
	#git push ../BackupOPolyglot/OPolyglot
	$(CPP) -Wall -std=c++11 -pthread -Wl,--no-as-needed -fPIC -Wno-unused-result \
	$(WX_LIBS)  $(OPTIONS) $(DEBUG_OPTIONS) -std=c++17 -Wextra -lstdc++ -ltomcrypt  build/obj/* \
	-o OPolyglot

build/obj/GuiOPolyglot.o: src/GuiOPolyglot.cpp src/GuiOPolyglot.cpp
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/GuiOPolyglot.cpp -o build/obj/GuiOPolyglot.o

build/obj/OPolyglotFullscreenFrame.o: src/OPolyglotFullscreenFrame.cpp src/OPolyglotFullscreenFrame.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotFullscreenFrame.cpp -o build/obj/OPolyglotFullscreenFrame.o

build/obj/OPolyglotThread.o: src/OPolyglotThread.cpp src/OPolyglotThread.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotThread.cpp -o build/obj/OPolyglotThread.o

build/obj/OPolyglot.o: src/OPolyglot.cpp src/OPolyglot.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglot.cpp -o build/obj/OPolyglot.o

build/obj/Utils.o: src/Utils.cpp src/Utils.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/Utils.cpp -o build/obj/Utils.o

build/obj/MainOPolyglot.o: src/MainOPolyglot.cpp src/MainOPolyglot.h src/Version.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/MainOPolyglot.cpp -o build/obj/MainOPolyglot.o

build/obj/OPolyglotSetup.o: src/OPolyglotSetup.cpp src/OPolyglotSetup.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotSetup.cpp -o build/obj/OPolyglotSetup.o

build/obj/OPolyglotDownloadLanguage.o: src/OPolyglotDownloadLanguage.cpp src/OPolyglotDownloadLanguage.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotDownloadLanguage.cpp -o build/obj/OPolyglotDownloadLanguage.o


build/obj/OPolyglotEvent.o: src/OPolyglotEvent.cpp src/OPolyglotEvent.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotEvent.cpp -o build/obj/OPolyglotEvent.o


build/obj/OPolyglotType.o: src/OPolyglotType.cpp src/OPolyglotType.h
	$(CPP) -Wall -fPIC $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotType.cpp -o build/obj/OPolyglotType.o


build/obj/OPolyglotTaskBar.o: src/OPolyglotTaskBar.cpp src/OPolyglotTaskBar.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotTaskBar.cpp -o build/obj/OPolyglotTaskBar.o

build/obj/OPolyglotProcessingRules.o: src/OPolyglotProcessingRules.cpp src/OPolyglotProcessingRules.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotProcessingRules.cpp -o build/obj/OPolyglotProcessingRules.o



build/obj/OPolyglotDynamic.o: src/OPolyglotDynamic.cpp 
	$(CPP) -Wall -fPIC $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) \
	-I build/src/translations/inference/marian-fork/src/3rd_party/ \
	-I build/src/translations/inference/src/ \
	-I ./build/src/translations/inference/marian-fork/src/ \
	-I ./build/src/translations/inference \
	-I ./build/src/translations/inference/3rd_party/ssplit-cpp/src/ssplit/ \
	-Wno-sign-compare -Wno-return-type -Wno-reorder -Wno-unused-value -Wno-deprecated-declarations \
	-Wno-template-id-cdtor -Wno-comment -Wno-unknown-pragmas \
	-fPIC -c src/OPolyglotDynamic.cpp -o build/obj/OPolyglotDynamic.o

libtranslator: build/obj build/obj/OPolyglotDynamic.o build/obj/OPolyglotType.o
	$(CPP) $(DEBUG_OPTIONS) -shared -Wall -std=c++11 -pthread  -Wl,--error-unresolved-symbols -Wl,--fatal-warnings -Wl,--no-as-needed -fPIC $(OPTIONS) $(WX_LIBS) $(TESSERACT_LIBS) $(BERGAMOT_LIBS) build/obj/OPolyglotDynamic.o build/obj/OPolyglotType.o -o build/libopolyglot-ocr-translator.so 
	rm build/obj/OPolyglotDynamic.o
	cp build/libopolyglot-ocr-translator.so bin


build/obj:
	mkdir -p build/obj
