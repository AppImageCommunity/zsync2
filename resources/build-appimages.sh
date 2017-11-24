#! /bin/bash

set -x
set -e

# use RAM disk if possible
if [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR=$(mktemp -d -p "$TEMP_BASE" zsync2-build-XXXXXX)

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT=$(readlink -f $(dirname $(dirname $0)))
OLD_CWD=$(readlink -f .)

pushd "$BUILD_DIR"

# TODO: fix setting those variables in the CMake configurations
cmake "$REPO_ROOT" -DBUILD_CPR_TESTS=OFF -DUSE_SYSTEM_CURL=ON -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo

# create AppDir
mkdir -p AppDir

# now, compile and install to AppDir
make -j$(nproc) install DESTDIR=AppDir

# install resources into AppDir
mkdir -p AppDir/usr/share/{applications,icons/hicolor/256x256/apps/} AppDir/resources
cp -v "$REPO_ROOT"/resources/*.desktop AppDir/usr/share/applications/
cp -v "$REPO_ROOT"/resources/*.png AppDir/usr/share/icons/hicolor/256x256/apps/

# determine Git commit ID
# linuxdeployqt uses this for naming the file
export VERSION=$(cd "$REPO_ROOT" && git rev-parse --short HEAD)

# prepend Travis build number if possible
if [ "$TRAVIS_BUILD_NUMBER" != "" ]; then
    export VERSION="$TRAVIS_BUILD_NUMBER-$VERSION"
fi

# get linuxdeployqt
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage

# bundle applications
./linuxdeployqt-continuous-x86_64.AppImage AppDir/usr/share/applications/zsync2.desktop -verbose=1 -bundle-non-qt-libs \
    -executable=AppDir/usr/bin/zsyncmake2

# remove some libraries which produce segfaults atm
find AppDir -type f -iname '*.a' -delete
rm -rf AppDir/usr/include

# get appimagetool
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage

# create zsync2 AppImage
./appimagetool-x86_64.AppImage -v --exclude-file "$REPO_ROOT"/resources/zsync2.ignore AppDir \
    -u 'gh-releases-zsync|TheAssassin|zsync2|continuous|zsync2-*x86_64.AppImage.zsync'

# change AppDir root to fit the GUI
pushd AppDir
rm AppRun && ln -s usr/bin/zsyncmake2 AppRun
rm *.desktop && cp usr/share/applications/zsyncmake2.desktop .
popd

# create zsyncmake2 AppImage
./appimagetool-x86_64.AppImage -v --exclude-file "$REPO_ROOT"/resources/zsyncmake2.ignore AppDir \
    -u 'gh-releases-zsync|TheAssassin|zsync2|continuous|zsyncmake2-*x86_64.AppImage.zsync'

# move AppImages to old cwd
mv zsync*2*.AppImage* "$OLD_CWD"/

popd
