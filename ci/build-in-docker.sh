#! /bin/bash

if [[ "$ARCH" == "" ]]; then
    echo "Usage: env ARCH=... bash $0"
    exit 1
fi

set -euxo pipefail

case "$ARCH" in
    x86_64)
        DOCKER_ARCH=amd64
        ;;
    i686)
        CMAKE_ARCH=i386
        DOCKER_ARCH=i386
        ;;
    armhf)
        DOCKER_ARCH=arm32v7
        ;;
    aarch64)
        DOCKER_ARCH=arm64v8
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 2
esac

CMAKE_ARCH="${CMAKE_ARCH:-"$ARCH"}"

cwd="$PWD"
repo_root="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")"/..)"

image=zsync2-build:"$ARCH"

# building local image to "cache" installed dependencies for subsequent builds
docker build \
    "${tty_args[@]}" \
    -t "$image" \
    --build-arg ARCH="$ARCH" \
    --build-arg DOCKER_ARCH="$DOCKER_ARCH" \
    --build-arg CMAKE_ARCH="$CMAKE_ARCH" \
    "$repo_root"/ci

tty_args=()
if [ -t 0 ]; then tty_args+=("-t"); fi

# run the build with the current user to
#   a) make sure root is not required for builds
#   b) allow the build scripts to "mv" the binaries into the /out directory
uid="$(id -u)"

# mount workspace read-only, trying to make sure the build doesn't ever touch the source code files
# of course, this only works reliably if you don't run this script from that directory
# but it's still not the worst idea to do so
docker run \
    --rm \
    -i \
    "${tty_args[@]}" \
    -e CI=1 \
    -e GITHUB_RUN_NUMBER \
    -v "$repo_root":/ws:ro \
    -v "$cwd":/out \
    -w /out \
    --user "$uid" \
    "$image" \
    bash /ws/ci/build-appimages.sh
