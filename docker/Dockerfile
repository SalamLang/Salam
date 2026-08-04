# syntax=docker/dockerfile:1
#
# Salam runtime image - minimal, downloads a prebuilt release (binary +
# std/) instead of compiling from source. See c/docker/Dockerfile for the
# from-source build used in development.
#
# `salam exec` (interpreter) and cross builds (`salam build --target=
# x86_64-linux-musl`, `...-w64-windows-gnu`, etc.) need zero extra
# toolchain - LLVM+LLD plus musl/mingw sysroots are statically embedded in
# the release binary. Building for the native/glibc host target (`salam
# build` with no --target) does shell out to a C compiler to link against
# glibc, so this image bundles tcc for that (see below). std/db
# (sqlite3/hiredis/mariadb) and std/ssl embed their libs statically too.
# The one std package NOT covered is std/webview, which dynamically links
# system gtk3/webkit2gtk and is out of scope for this minimal image.

# trixie-slim (glibc 2.41), not bookworm-slim (glibc 2.36): the release
# binary is built on a GitHub-hosted runner whose glibc is newer than
# bookworm ships, so bookworm-slim fails at runtime with a GLIBC_2.38
# version-not-found error.
ARG DEBIAN_VERSION=trixie-slim

FROM debian:${DEBIAN_VERSION} AS fetch
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl unzip \
    && rm -rf /var/lib/apt/lists/*

ARG SALAM_VERSION=""
ENV SALAM_VERSION=${SALAM_VERSION}
COPY install.sh /tmp/install.sh
RUN sh /tmp/install.sh --dir /opt/salam && rm -f /tmp/install.sh

FROM debian:${DEBIAN_VERSION} AS runtime
# trivy:ignore:AVD-DS-0025
# tcc + libc6-dev (~24MB combined) are for `salam build` with no --target
# (the native/glibc host target): unlike cross targets (e.g.
# --target=x86_64-linux-musl), which link via salam's embedded LLD against
# an embedded musl sysroot and need nothing extra, the native glibc target
# needs a real C compiler plus glibc's crt*.o/headers to link against -
# there's no statically-embeddable glibc sysroot. tcc (not gcc) keeps this
# small since salam only needs a C89 compiler to build its own C output.
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates libstdc++6 libtinfo6 libxml2 libzstd1 zlib1g tcc libc6-dev \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd --gid 10001 salam \
    && useradd --uid 10001 --gid salam --no-create-home --shell /usr/sbin/nologin salam \
    && mkdir -p /work && chown salam:salam /work \
    && rm -f /usr/bin/apt* /usr/bin/dpkg* 2>/dev/null || true

COPY --from=fetch /opt/salam /opt/salam

ENV SALAM_STD=/opt/salam/std \
    SALAM_CACHE=/tmp \
    PATH=/opt/salam:$PATH

WORKDIR /work
USER salam

HEALTHCHECK --interval=5m --timeout=10s --start-period=5s --retries=3 \
    CMD ["salam", "version"]

ENTRYPOINT ["salam"]
CMD ["help"]
