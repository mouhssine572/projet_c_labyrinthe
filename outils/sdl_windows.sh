#!/bin/sh
# Télécharge les versions Windows (MinGW, 64 bits) de SDL2, SDL2_ttf et SDL2_mixer,
# publiées par les auteurs de SDL sur GitHub, et les range dans le dossier donné :
#   <dossier>/include/SDL2/*.h   les en-têtes
#   <dossier>/lib/*.a            les bibliothèques pour l'édition de liens
#   <dossier>/bin/*.dll          les DLL à livrer à côté du .exe
# Utilisé par « make windows » ; il n'y a normalement pas besoin de le lancer à la main.
set -e

DESTINATION=${1:-build/windows/sdl}
SDL_VERSION=2.32.10
TTF_VERSION=2.24.0
MIXER_VERSION=2.8.1

TEMPORAIRE=$(mktemp -d)
trap 'rm -rf "$TEMPORAIRE"' EXIT
mkdir -p "$DESTINATION"

# telecharger <dépôt> <archive> <version>
telecharger() {
    url="https://github.com/libsdl-org/$1/releases/download/release-$3/$2-devel-$3-mingw.tar.gz"
    echo "Téléchargement de $2 $3..."
    curl -fsSL "$url" -o "$TEMPORAIRE/$2.tar.gz"
    mkdir -p "$TEMPORAIRE/$2"
    tar -xzf "$TEMPORAIRE/$2.tar.gz" -C "$TEMPORAIRE/$2" --strip-components=1
    cp -r "$TEMPORAIRE/$2/x86_64-w64-mingw32/." "$DESTINATION/"
}

telecharger SDL       SDL2       "$SDL_VERSION"
telecharger SDL_ttf   SDL2_ttf   "$TTF_VERSION"
telecharger SDL_mixer SDL2_mixer "$MIXER_VERSION"
echo "Bibliothèques SDL pour Windows prêtes dans $DESTINATION"
