OPTIONS=-g
CPP=g++-14
WX_CFLAGS=$(shell wx-config --cxxflags)
WX_LIBS=$(shell wx-config --libs)
#$(shell pkg-config --libs valgrind)
BERGAMOTH_PATH=/home/oleksandr/tmp/build/bergamot-translator/src/
BERGAMOTG_ROOT_PATH=/home/oleksandr/tmp/build/bergamot-translator
MARIAN_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/marian-dev/src/
MARIAN_DEP_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/marian-dev/src/3rd_party/
SPLIT_PATH=/home/oleksandr/tmp/build/bergamot-translator/3rd_party/ssplit-cpp/src/ssplit/
#MARIAN_LIB=-L./build/bergamot-translator/build -lmarian -lssplit \
		   	build/bergamot-translator/build/3rd_party/marian-dev/src/3rd_party/yaml-cpp/CMakeFiles/libyaml-cpp.dir/*.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/graph/*.cpp.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/common/*.cpp.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/3rd_party/phf/*.cc.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/data/sentencepiece_vocab.cpp.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/data/vocab.cpp.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/data/*.cpp.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/translator/*.cpp.o \
			build/bergamot-translator/build/3rd_party/marian-dev/src/3rd_party/sentencepiece/src/CMakeFiles/sentencepiece-static.dir/*.cc.o \
			build/bergamot-translator/build/3rd_party/ssplit-cpp/src/CMakeFiles/ssplit.dir/ssplit/ssplit.cpp.o \
			build/bergamot-translator/build/3rd_party/ssplit-cpp/src/CMakeFiles/ssplit.dir/ssplit/regex.cpp.o \
			build/bergamot-translator/build/src/translator/CMakeFiles/bergamot-translator.dir/*.cpp.o
NARIAN_LIB=-L./build/lib64 -lyaml-cpp 
#-L./build/lib64 -lmarian -lsentencepiece -lsentencepiece_train -lyaml-cpp
#BERGAMOT_LIB=-L./build/bergamot-translator/build/src/translator/ -lbergamot-translator  \
	-L ./build/bergamot-translator/build/ -lmarian \
	./build/bergamot-translator/build/3rd_party/marian-dev/src/3rd_party/yaml-cpp/CMakeFiles/libyaml-cpp.dir/*.cpp.o	\
	./build/bergamot-translator/build/src/translator/CMakeFiles/bergamot-translator.dir/*.cpp.o \
	./build/bergamot-translator/build/3rd_party/marian-dev/src/CMakeFiles/marian.dir/graph/*.cpp.o

BERGAMOT_LIBS=-L./build/lib -lmarian-shared -lbergamot-translator-shared 
TESSERACT_LIBS=-ltesseract -larchive -lcurl -lleptonica
TRANSLATOR_LIB=-Lbuild/ -ltranslator

test1:
	echo "test"

test2: test1

all:
	# test
	echo "make build"

configLIBRARY:
	export LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH

clean:
	git bundle create ../BackupOPolyglot.bundle --all
	rm -r build/obj
	rm OPolyglot
	
build: build/obj build/obj/MainOPolyglot.o build/obj/GuiOPolyglot.o build/obj/OPolyglot.o    build/obj/OPolyglotDownloadLanguage.o build/obj/OPolyglotSetup.o build/obj/Utils.o
	#git push ../BackupOPolyglot/OPolyglot
	$(CPP) -Wall -std=c++11 -pthread -Wl,--no-as-needed -fPIC -Wno-unused-result \
	$(WX_LIBS)  $(TESSERACT_LIBS) $(OPTIONS) -std=c++17 -Wextra -lstdc++ -ltomcrypt  build/obj/* \
	-o OPolyglot

build/obj/GuiOPolyglot.o: src/GuiOPolyglot.cpp src/GuiOPolyglot.cpp
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) -c src/GuiOPolyglot.cpp -o build/obj/GuiOPolyglot.o

build/obj/OPolyglot.o: src/OPolyglot.cpp src/OPolyglot.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) -c src/OPolyglot.cpp -o build/obj/OPolyglot.o

build/obj/Utils.o: src/Utils.cpp src/Utils.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) -c src/Utils.cpp -o build/obj/Utils.o

build/obj/MainOPolyglot.o: src/MainOPolyglot.cpp src/MainOPolyglot.h src/Version.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) -c src/MainOPolyglot.cpp -o build/obj/MainOPolyglot.o

build/obj/OPolyglotSetup.o: src/OPolyglotSetup.cpp src/OPolyglotSetup.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) -c src/OPolyglotSetup.cpp -o build/obj/OPolyglotSetup.o

build/obj/OPolyglotDownloadLanguage.o: src/OPolyglotDownloadLanguage.cpp src/OPolyglotDownloadLanguage.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) -c src/OPolyglotDownloadLanguage.cpp -o build/obj/OPolyglotDownloadLanguage.o

build/obj/Translator.o: src/Translator.cpp src/Translator.h
	$(CPP) -Wall -fPIC $(WX_CFLAGS) $(OPTIONS) \
	-I build/src/translations/inference/marian-fork/src/3rd_party/ \
	-I build/src/translations/inference/src/ \
	-I ./build/src/translations/inference/marian-fork/src/ \
	-I ./build/src/translations/inference \
	-I ./build/src/translations/inference/3rd_party/ssplit-cpp/src/ssplit/ \
	-Wno-sign-compare -Wno-return-type -Wno-reorder -Wno-unused-value -Wno-deprecated-declarations \
	-Wno-template-id-cdtor -Wno-unknown-pragmas -Wno-comment \
	-c src/Translator.cpp -o build/obj/Translator.o
	$(CPP) --shared -Wall -std=c++11 -pthread  -Wl,--no-as-needed -fPIC $(WX_LIBS) $(BERGAMOT_LIBS) build/obj/Translator.o -o build/libtranslator.so -Wl,--out-implib,build/libtranslator.a

build/obj/OPolyglotDynamic.o: src/OPolyglotDynamic.cpp src/OPolyglotDynamic.h
	$(CPP) -Wall -fPIC $(WX_CFLAGS) $(OPTIONS) \
	-I build/src/translations/inference/marian-fork/src/3rd_party/ \
	-I build/src/translations/inference/src/ \
	-I ./build/src/translations/inference/marian-fork/src/ \
	-I ./build/src/translations/inference \
	-I ./build/src/translations/inference/3rd_party/ssplit-cpp/src/ssplit/ \
	-Wno-sign-compare -Wno-return-type -Wno-reorder -Wno-unused-value -Wno-deprecated-declarations \
	-Wno-template-id-cdtor -Wno-unknown-pragmas -Wno-comment \
	-c src/OPolyglotDynamic.cpp -o build/obj/OPolyglotDynamic.o
	$(CPP) -shared -Wall -std=c++11 -pthread  -Wl,--no-as-needed -fPIC $(WX_LIBS) $(BERGAMOT_LIBS) build/obj/OPolyglotDynamic.o -o build/libOPolyglotTranslator.so -Wl,--out-implib,build/libOPolyglotTranslator.a
	rm build/obj/OPolyglotDynamic.o

#build/libtranslator.a: build/obj/Translator.o

libtranslator: build/obj build/obj/OPolyglotDynamic.o
	echo "BUILD shared libtranslator"
	
	


build/obj:
	mkdir -p build/obj
