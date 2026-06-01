# OPolyglot
##  Your Private Offline Translator 
OPolyglot is a fast and fully offline translator that respects your privacy. The application is powered by the robust "Bergamot" engine, 
allowing you to perform all translations locally on your device without the need for an internet connection.
OPolyglot is an excellent assistant for reading foreign documents, playing games, or working with unlocalized applications, all while keeping your data completely secure.

### Release v0.2.0:
* **Add Only OCR mode**
* **Added image pre-processing**
* **Add translation editor**
* **Add README.md viewer**

At the current stage of development, OPolyglot offers three key features:
* **Language Management**: Automatic downloading of language models via `libcurl`.
* **Simple Translator**: A convenient tool for classic translation of typed or copied text.
* **Screen Translator**: A smart feature that allows you to take a screenshot of any screen area, automatically recognize the text (OCR), and instantly translate it.


## Install
* AppImage:
    * Download [OPolyglot-x86_64.AppImage](https://sourceforge.net/projects/opolyglot/files/v0.2.0/OPolyglot-x86_64.AppImage/download)
    * Run `./OPolyglot-x86_64.AppImage`
* Flatpak:
    * Download [OPolyglot.flatpak](https://sourceforge.net/projects/opolyglot/files/v0.2.0/OPolyglot.flatpak/download)
	* Configure flathub: `flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo`
    * Install via terminal: `flatpak install OPolyglot.flatpak`
* Snap:
    * Run: `sudo snap install opolyglot`
* Windows:
    * Download the [Installer (v0.2.0)](https://sourceforge.net/projects/opolyglot/files/v0.2.0/opsetup.exe/download)
    * Run `opsetup.exe` to install the application.

## [Source Code](https://github.com/fenenko/OPolyglot)



