# OPolyglot
##  Your Private Offline Translator 
**version v0.3.0**

OPolyglot is a fast and fully offline translator that respects your privacy. The application is powered by the robust "Bergamot" engine, 
allowing you to perform all translations locally on your device without the need for an internet connection.
OPolyglot is an excellent assistant for reading foreign documents, playing games, or working with unlocalized applications, all while keeping your data completely secure.


### Release v0.3.0:
* **A PDF document translator has been implemented**
* **Reduced CPU requirement for Linux build in libmarian to Core2 level**

### Release v0.2.2:
* **Fixed app crash when opening the translator twice**

### Release v0.2.1:
* **Fix crash in Flatpak application**

### Release v0.2.0:
* **Add Only OCR mode**
* **Added image pre-processing**
* **Add translation editor**
* **Add README.md viewer**

At the current stage of development, OPolyglot offers three key features:
* **Language Management**: Automatic downloading of language models via `libcurl`.
* **Document Translator**: The app helps translate PDF documents.
* **Simple Translator**: A convenient tool for classic translation of typed or copied text.
* **Screen Translator**: A smart feature that allows you to take a screenshot of any screen area, automatically recognize the text (OCR), and instantly translate it.


## Install
* AppImage:
    * Download [OPolyglot-x86_64.AppImage](https://sourceforge.net/projects/opolyglot/files/v0.3.0/OPolyglot-x86_64.AppImage/download)
    * Run `./OPolyglot-x86_64.AppImage`
* Flatpak:
    * Download [opolyglot-x86_64.flatpak](https://sourceforge.net/projects/opolyglot/files/v0.3.0/opolyglot-x86_64.flatpak/download)
	* Configure flathub: `flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo`
    * Install via terminal: `flatpak install opolyglot-x86_64.flatpak`
* [Snap](https://snapcraft.io/opolyglot):
    * Run: `sudo snap install opolyglot`
* Windows:
    * Download the [Installer (v0.3.0)](https://sourceforge.net/projects/opolyglot/files/v0.3.0/opsetup.exe/download)
    * Run `opsetup.exe` to install the application.

## [Source Code](https://github.com/fenenko/OPolyglot)

## Build from source code

### Build on linux

download source code

`git clone https://github.com/fenenko/OPolyglot`

`cd OPolyglot`


build dependencies

`cd scripts`

`./simple-build-dependencies.sh`

`cd ..`


build OPolyglot

`make build`


run OPolyglot

`export LD_LIBRARY_PATH=$(readlink -f ./bin):$LD_LIBRARY_PATH`

`cd bin`

`./opolyglot`

### Cross-build for Windows on Linux(using mingw)

download source code

`git clone https://github.com/fenenko/OPolyglot`

`cd OPolyglot`


build dependencies

`cd scripts`

`./simple-build-library-win.sh`

`cd ..`


build OPolyglot

`make MINGW=1 build`


run OPolyglot using wine

`cd bin`

`wine ./opolyglot`



