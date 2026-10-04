FROM arm32v7/debian:sid

LABEL org.opencontainers.image.title="Salam armhf builder"
LABEL org.opencontainers.image.description="arm32v7 toolchain plus a static libxml2 for building Salam with embedded LLVM+LLD"

ARG LIBXML2_TAG=v2.15.4

# hadolint ignore=DL3008
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential ca-certificates ccache file git cmake ninja-build \
        llvm-22-dev liblld-22-dev libpolly-22-dev libzstd-dev zlib1g-dev \
        libtinfo-dev libxml2-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --quiet --depth 1 --branch "${LIBXML2_TAG}" https://github.com/GNOME/libxml2.git /tmp/libxml2-src \
    && cmake /tmp/libxml2-src -G Ninja -B /tmp/libxml2-build \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
        -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_TESTS=OFF \
        -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_HTTP=OFF \
        -DLIBXML2_WITH_FTP=OFF -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_ZLIB=ON \
    && ninja -C /tmp/libxml2-build \
    && mkdir -p /opt/libxml2-static \
    && cp /tmp/libxml2-build/libxml2.a /opt/libxml2-static/ \
    && rm -rf /tmp/libxml2-src /tmp/libxml2-build

RUN useradd --create-home --shell /bin/bash --uid 1001 builder

ENV HOME=/tmp

HEALTHCHECK NONE

WORKDIR /work
USER builder
