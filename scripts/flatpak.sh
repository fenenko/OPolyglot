#!/bin/bash
set -eux

#export ALL_PROXY=socks5://127.0.0.1:9050
FLATHUB=https://flathub.org/repo/flathub.flatpakrepo

# needed in github-ci
if [ `id -u` == 0 ]; then
  command -v dnf && { dnf upgrade -y ; dnf install -y flatpak-builder git wget; }
fi


mkdir -p ../build/flatpak
cp io.sourceforge.opolyglot.OPolyglot.json ../build/flatpak
cp FlatpakInference.patch ../build/flatpak
cd ../build/flatpak

NOTFOUND=""
command -v flatpak         >/dev/null 2>&1 || NOTFOUND="$NOTFOUND flatpak"
command -v flatpak-builder >/dev/null 2>&1 || NOTFOUND="$NOTFOUND flatpak-builder"
command -v git             >/dev/null 2>&1 || NOTFOUND="$NOTFOUND git"
command -v wget            >/dev/null 2>&1 || NOTFOUND="$NOTFOUND wget"
test -z "$NOTFOUND" || { echo "$NOTFOUND: not found, but needed. please install. Aborting. "; exit 1; }

flatpak remote-add --if-not-exists flathub $FLATHUB
flatpak install -y flathub org.freedesktop.Platform 23.08
flatpak install -y flathub org.freedesktop.Sdk 23.08
#flatpak-builder --force-clean --repo=repo ./build io.sourceforge.opolyglot.OPolyglot.json
flatpak-builder --force-clean --repo=repo ./build io.sourceforge.opolyglot.OPolyglot.json
flatpak build-bundle repo OPolyglot.flatpak io.sourceforge.OPolyglot --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
#flatpak-builder --force-clean --repo=repo ./build io.test.json
#flatpak build-bundle repo wxFormBuilder.flatpak org.wxformbuilder.wxFormBuilder --runtime-repo=$FLATHUB
