# syntax=docker/dockerfile:1
#
# Salam runtime image - minimal, downloads a prebuilt release (binary +
# std/) instead of compiling from source. See c/docker/Dockerfile for the
# from-source build used in development.
#
# The published release binary embeds LLVM+LLD statically, so both
# `salam exec` (interpreter) and `salam build`/`salam llvm` (native/JIT
# codegen and linking) work with zero extra system toolchain - this image
# needs no gcc/cc/ld. std/db (sqlite3/hiredis/mariadb) and std/ssl also
# embed their libs statically. The one std package NOT covered is
# std/webview, which dynamically links system gtk3/webkit2gtk and is out of
# scope for this minimal image.

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
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates libstdc++6 libtinfo6 libxml2 libzstd1 zlib1g \
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
