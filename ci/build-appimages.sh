#! /bin/bash

set -euxo pipefail

if [ "$ARCH" == "" ]; then
    echo "Usage: env ARCH=... bash $0"
    exit 2
fi

# use RAM disk if possible
# in Docker, /dev/shm is typically mounted with noexec, so one can't run executable stored in there
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" zsync2-build-XXXXXX)"

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT="$(readlink -f "$(dirname "$(dirname "$0")")")"
OLD_CWD="$(readlink -f .)"

pushd "$BUILD_DIR"

cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo

# create AppDir
mkdir -p AppDir

# now, compile and install to AppDir
make -j"$(nproc)"
make install DESTDIR=AppDir

LD_ARCH="$ARCH"
if [ "$ARCH" == "i686" ]; then LD_ARCH="i386"; fi

# get linuxdeploy
wget https://github.com/TheAssassin/linuxdeploy/releases/download/continuous/linuxdeploy-"$LD_ARCH".AppImage
chmod +x linuxdeploy*.AppImage

patch_appimage() {
    while [[ "${1:-}" != "" ]]; do
        dd if=/dev/zero of="$1" conv=notrunc bs=1 count=3 seek=8
        shift
    done
}
patch_appimage linuxdeploy-"$LD_ARCH".AppImage

# determine Git commit ID
# linuxdeploy uses this for naming the file
export VERSION=$(cd "$REPO_ROOT" && git rev-parse --short HEAD)

# prepend GitHub actions run number if possible
if [ "${GITHUB_RUN_NUMBER:-}" != "" ]; then
    export VERSION="$GITHUB_RUN_NUMBER-$VERSION"
fi

for app in zsync2 zsyncmake2; do
    # prepare for next app
    pushd AppDir
    rm AppRun *.desktop || true
    popd

    # prepare AppDir with linuxdeploy and create AppImage
    export UPD_INFO="gh-releases-zsync|AppImage|zsync2|continuous|$app-*x86_64.AppImage.zsync"

    ./linuxdeploy-"$LD_ARCH".AppImage --appdir AppDir \
        -d "$REPO_ROOT"/resources/"$app".desktop \
        -i "$REPO_ROOT"/resources/zsync2.svg \
        --output appimage
done

# move AppImages to old cwd
mv zsync*2*.AppImage* "$OLD_CWD"/

popd
