ARG DOCKER_ARCH

# current Debian oldstable as of Dec 2023
# we use Debian because they still provide i386 builds
FROM ${DOCKER_ARCH}/debian:bullseye

ARG ARCH
ARG DOCKER_ARCH
ARG CMAKE_ARCH

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
        gcc g++ make libxpm-dev git libcurl4-openssl-dev libssl-dev wget zlib1g-dev libc6-dev bsdmainutils pkgconf libgcrypt20-dev ca-certificates file libglib2.0-0

RUN wget https://artifacts.assassinate-you.net/prebuilt-cmake/cmake-v3.28.0-debian-bullseye-${CMAKE_ARCH}.tar.gz -O- | \
        tar xz -C /usr/local --strip-components=1

COPY ./install-gtest.sh /
RUN bash /install-gtest.sh

COPY libgcrypt.pc /usr/lib/i386-linux-gnu/pkgconfig/libgcrypt.pc
RUN triplet="$(find /usr/lib/ -maxdepth 1 -type d -iname '*-linux-gnu*' | head -n1 | rev | cut -d/ -f1 | rev)" && \
    sed -i "s|x86_64-linux-gnu|${triplet}|g" /usr/lib/*/pkgconfig/libgcrypt.pc

# work around bug in FindCURL.cmake, which does not parse the pkg-config provided protocols and features into lists causing
# the comparison in the loop to yield false negative results
# this makes it use curl-config which works much better
RUN rm /usr/lib/*/pkgconfig/libcurl.pc

ENV APPIMAGE_EXTRACT_AND_RUN=1

ENV ARCH=${ARCH}
