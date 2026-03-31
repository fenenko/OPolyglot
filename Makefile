.PHONY: flatpak flatpak-clean flatpak-sh snap snap-clean snapcraft-set-core18 linux

OPTIONS=-g 
CPP=g++
WX_CFLAGS=$(shell wx-config --cxxflags base,core,net,xml,stc)
WX_LIBS=$(shell wx-config --libs base,core,net,xml,stc)
OUTPUT_LIB=libopolyglot-ocr-translator.so
OPTIONS_LIB=-fPIC
TESSERACT_LIBS=-ltesseract 
TOMCRYPT=-ltomcrypt
BERGAMOT_INC=-Ibuild/linux/include/inference/src -Ibuild/linux/include/inference/marian-fork/src/ -Ibuild/linux/include/inference/marian-fork/src/3rd_party/ -Ibuild/linux/include/inference/ -Ibuild/linux/include/inference/3rd_party/ssplit-cpp/src/ssplit/
BERGAMOT_LIBS=-Lbuild/linux/bin -lmarian -lbergamot-translator-source
PORTAL_CFLAGS=$(shell pkg-config --cflags libportal,libportal-gtk3)
PORTAL_LIBS=$(shell pkg-config --libs libportal,libportal-gtk3)
ifeq ($(SNAP), 1)
CPP=g++-13
BERGAMOT_INC=-I$(SNAPCRAFT_STAGE)/bergamot/inference/src -I$(SNAPCRAFT_STAGE)/bergamot/inference/marian-fork/src/ -I$(SNAPCRAFT_STAGE)/bergamot/inference/marian-fork/src/3rd_party/ -I$(SNAPCRAFT_STAGE)/bergamot/inference/ -I$(SNAPCRAFT_STAGE)/bergamot/inference/3rd_party/ssplit-cpp/src/ssplit/
BERGAMOT_LIBS=-L$(SNAPCRAFT_STAGE)/usr/lib/$(CRAFT_ARCH_TRIPLET_BUILD_FOR) -lmarian -lbergamot-translator-source
WX_CFLAGS=$(shell $(SNAPCRAFT_STAGE)/usr/lib/wx/config/gtk3-unicode-3.2 --prefix=$(SNAPCRAFT_STAGE)/usr --cxxflags)
WX_LIBS=$(shell $(SNAPCRAFT_STAGE)/usr/lib/wx/config/gtk3-unicode-3.2 --prefix=$(SNAPCRAFT_STAGE)/usr --libs base,core,net,xml,stc)
OPTIONS = -D__SNAP
else ifeq ($(FLATPAK), 1)
BERGAMOT_INCLUDE_SOURCE=./inference
BERGAMOT_INCLUDE_DEST=/app/include
BERGAMOT_INC=-I/app/include/inference/src -I/app/include/inference/marian-fork/src/ -I/app/include/inference/marian-fork/src/3rd_party/ -I/app/include/inference/ -I/app/include/inference/3rd_party/ssplit-cpp/src/ssplit/
BERGAMOT_LIBS=-lmarian -lbergamot-translator-source
OPTIONS = -D__FLATPAK
else ifeq ($(MINGW),1)
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
PORTAL_CFLAGS =
PORTAL_LIBS =
else
VERSION_HDR := src/OPolyglotVersion.h
OPOLYGLOT_VERSION_NAME := $(shell sed -n 's/^[[:space:]]*#define[[:space:]]\+OPOLYGLOT_VERSION_NAME[[:space:]]\+\(.*\)/\1/p' $(VERSION_HDR) | sed -e 's/^[[:space:]]*"\(.*\)"[[:space:]]*$$/\1/')
OPOLYGLOT_VERSION_MINOR := $(shell sed -n 's/^[[:space:]]*#define[[:space:]]\+OPOLYGLOT_VERSION_MINOR[[:space:]]\+\(.*\)/\1/p' $(VERSION_HDR) | sed -e 's/^[[:space:]]*"\(.*\)"[[:space:]]*$$/\1/')
endif



bin:
	mkdir -p bin

all:
	echo "make sanitize-mem"
	echo "make valgrind-mem"

help: all
	@echo "make MINGW=1 build"
	@echo "make FLATPAK=1 build"
	@echo "make SNAP=1 build"

opolyglot:	build

clean: 
	rm -rf build/obj/*
	rm -rf bin/*


gettext:
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


	
build: bin build/obj build/obj/MainOPolyglot.o build/obj/GuiOPolyglot.o build/obj/OPolyglot.o build/obj/OPolyglotDownloadLanguage.o build/obj/OPolyglotSetup.o build/obj/Utils.o build/obj/OPolyglotFullscreenFrame.o build/obj/OPolyglotThread.o build/obj/OPolyglotEvent.o build/obj/OPolyglotTaskBar.o build/obj/OPolyglotProcessingRules.o build/obj/OPolyglotAbout.o 
	$(CPP) build/obj/* $(PORTAL_LIBS) $(WX_LIBS) $(TOMCRYPT) $(OPTIONS) -o bin/opolyglot
ifeq ($(SNAP), 1)
	@echo "------SNAP------"
else ifeq ($(FLATPAK), 1)
	@echo "----FLATPAK----"
else
	mkdir -p bin/res
	cp ./res/download.xml bin/res
endif
	@echo "-----------------------FINISH-----------------------------"
	@echo "$(WX_LIBS)"


build/obj/GuiOPolyglot.o: src/GuiOPolyglot.cpp src/GuiOPolyglot.cpp
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/GuiOPolyglot.cpp -o build/obj/GuiOPolyglot.o

build/obj/OPolyglotFullscreenFrame.o: src/OPolyglotFullscreenFrame.cpp src/OPolyglotFullscreenFrame.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotFullscreenFrame.cpp -o build/obj/OPolyglotFullscreenFrame.o

build/obj/OPolyglotThread.o: src/OPolyglotThread.cpp src/OPolyglotThread.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotThread.cpp -o build/obj/OPolyglotThread.o
	
build/obj/OPolyglotAbout.o: src/OPolyglotAbout.cpp src/OPolyglotAbout.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) $(TOMCRYPT_INC)  -c src/OPolyglotAbout.cpp -o build/obj/OPolyglotAbout.o

build/obj/OPolyglot.o: src/OPolyglot.cpp src/OPolyglot.h
	$(CPP) -Wall $(PORTAL_CFLAGS) $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglot.cpp -o build/obj/OPolyglot.o

build/obj/Utils.o: src/Utils.cpp src/Utils.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/Utils.cpp -o build/obj/Utils.o

build/obj/MainOPolyglot.o: src/MainOPolyglot.cpp src/MainOPolyglot.h src/Version.h
	@echo "$(WX_LIBS) $(WX_CFLAGS)"
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/MainOPolyglot.cpp -o build/obj/MainOPolyglot.o

build/obj/OPolyglotSetup.o: src/OPolyglotSetup.cpp src/OPolyglotSetup.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotSetup.cpp -o build/obj/OPolyglotSetup.o

build/obj/OPolyglotDownloadLanguage.o: src/OPolyglotDownloadLanguage.cpp src/OPolyglotDownloadLanguage.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) $(TOMCRYPT_INC) -c src/OPolyglotDownloadLanguage.cpp -o build/obj/OPolyglotDownloadLanguage.o


build/obj/OPolyglotEvent.o: src/OPolyglotEvent.cpp src/OPolyglotEvent.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotEvent.cpp -o build/obj/OPolyglotEvent.o


build/obj/OPolyglotTaskBar.o: src/OPolyglotTaskBar.cpp src/OPolyglotTaskBar.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotTaskBar.cpp -o build/obj/OPolyglotTaskBar.o

build/obj/OPolyglotProcessingRules.o: src/OPolyglotProcessingRules.cpp src/OPolyglotProcessingRules.h
	$(CPP) -Wall $(WX_CFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) -c src/OPolyglotProcessingRules.cpp -o build/obj/OPolyglotProcessingRules.o



build/obj/OPolyglotDynamic.o: src/OPolyglotDynamic.cpp 
	$(CPP) $(WX_CFLAGS) $(OPTIONS) $(OPTIONS_LIB) $(DEBUG_OPTIONS)  \
	-Wno-sign-compare -Wno-return-type -Wno-reorder -Wno-unused-value -Wno-deprecated-declarations \
	-Wno-template-id-cdtor -Wno-comment -Wno-unknown-pragmas -fPIC $(BERGAMOT_INC) \
	-c src/OPolyglotDynamic.cpp -o build/obj/OPolyglotDynamic.o

libtranslator: build/obj build/obj/OPolyglotDynamic.o 
	$(CPP)  $(OPTIONS) $(OPTIONS_LIB)  -shared  -Wl,--no-undefined -o bin/$(OUTPUT_LIB) build/obj/OPolyglotDynamic.o  $(WX_LIBS) $(BERGAMOT_LIBS) $(TESSERACT_LIBS)
ifeq ($(SNAP), 1)
	@echo "------SNAP------"
else ifeq ($(FLATPAK), 1)
	@echo "----FLATPAK----"
else
	@echo "DEFAULT"
	cp build/linux/bin/libmarian.so bin
	cp build/linux/bin/libbergamot-translator-source.so bin
endif
	rm build/obj/OPolyglotDynamic.o

RUNTIME = org.freedesktop.Platform
VERSION = 23.08
RUNTIME_FULL_ID = $(RUNTIME)//$(VERSION)
SDK = org.freedesktop.Sdk
SDK_FULL_ID = $(SDK)//$(VERSION)
SDK_VALA = org.freedesktop.Sdk.Extension.vala
SDK_VALA_FULL_ID = $(SDK_VALA)//$(VERSION)

flatpak-check-env:
	@echo "Checking the environment for $(RUNTIME_FULL_ID) , $(SDK_FULL_ID) , $(SDK_VALA_FULL_ID)"
	@flatpak info $(RUNTIME_FULL_ID) > /dev/null 2>&1 || $(MAKE) flatpak-install-runtime
	@flatpak info $(SDK_FULL_ID) > /dev/null 2>&1 || $(MAKE) flatpak-install-sdk
	@flatpak info $(SDK_VALA_FULL_ID) > /dev/null 2>&1 || $(MAKE) flatpak-install-sdk-vala

flatpak-install-runtime:
	@echo "Package $(RUNTIME_FULL_ID) not found. Installation..."
	flatpak install --user -y flathub $(RUNTIME_FULL_ID)

flatpak-install-sdk:
	@echo "Package $(SDK_FULL_ID) not found. Installation..."
	flatpak install --user -y flathub $(SDK_FULL_ID)

flatpak-install-sdk-vala:
	@echo "Package $(SDK_VALA_FULL_ID) not found. Installation..."
	flatpak install --user -y flathub $(SDK_VALA_FULL_ID)

flatpak-clean:
	rm -rf build/flatpak

flatpak: flatpak-check-env
	$(MAKE) -f Makefile flatpak-check-env
	mkdir -p build/flatpak/build
	mkdir -p build/flatpak/repo
	flatpak-builder --force-clean --state-dir=build/flatpak --repo=build/flatpak/repo build/flatpak/build flatpak/opolyglot.yaml
	flatpak build-bundle build/flatpak/repo opolyglot-proxima_centauri_1-x86_64.flatpak io.sourceforge.opolyglot --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo

flatpak-sh:
	flatpak-builder --run build/flatpak/build flatpak/opolyglot.yaml sh

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
	export LD_LIBRARY_PATH=$$(readlink -f bin):$$LD_LIBRARY_PATH && cd bin && ./opolyglot

linux:
	$(MAKE) build
	$(MAKE) libtranslator
	$(MAKE) translatormo

build/obj:
	mkdir -p build/obj
