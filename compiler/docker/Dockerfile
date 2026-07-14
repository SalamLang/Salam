# syntax=docker/dockerfile:1
#
# Salam compiler - multi-stage image (Alpine / musl, smallest practical base).
#
# Stages:
#   toolchain  - base image with everything needed to BUILD and RUN salam
#                (C compilers + LLVM 22 + make). Shared by every other stage.
#   dev        - live development: source is bind-mounted, salam is recompiled
#                on every change (see tools/bash/docker-dev.sh).
#   builder    - production build: COPY . then `make && make install`.
#   prod       - slim runtime: only the installed `salam` + std/ from `builder`,
#                on top of the same toolchain (salam shells out to cc/llvm at
#                run time, so the toolchain must be present to compile programs).
#

ARG ALPINE_VERSION=edge
ARG LLVM_VERSION=22

FROM alpine:${ALPINE_VERSION} AS toolchain
ARG LLVM_VERSION
SHELL ["/bin/ash", "-o", "pipefail", "-c"]

RUN for i in 1 2 3 4 5; do \
        apk add --no-cache \
            build-base \
            make \
            musl-dev \
            tcc \
            tcc-libs-static \
            clang${LLVM_VERSION} \
            llvm${LLVM_VERSION} \
            bash \
            git \
            sqlite-dev \
            sqlite-static \
            mariadb-connector-c-dev \
        && break; \
        echo "apk add attempt $i failed; retrying in 5s..." >&2; sleep 5; \
    done

RUN set -eux; \
    LLVM_BIN="/usr/lib/llvm${LLVM_VERSION}/bin"; \
    for t in clang llc opt lli; do \
        if   [ -x "$LLVM_BIN/$t" ]; then src="$LLVM_BIN/$t"; \
        elif command -v "$t-${LLVM_VERSION}" >/dev/null 2>&1; then src="$(command -v "$t-${LLVM_VERSION}")"; \
        elif command -v "$t" >/dev/null 2>&1; then src="$(command -v "$t")"; \
        else echo "missing LLVM tool: $t" >&2; exit 1; fi; \
        ln -sf "$src" "/usr/local/bin/$t-${LLVM_VERSION}"; \
    done; \
    clang-${LLVM_VERSION} --version; \
    llc-${LLVM_VERSION} --version | head -1

ARG SALAM_UID=1000
ARG SALAM_GID=1000
RUN addgroup -g "${SALAM_GID}" salam \
    && adduser -D -u "${SALAM_UID}" -G salam salam

ENV CC=gcc
WORKDIR /app

FROM toolchain AS dev
RUN apk add --no-cache entr
RUN chown -R salam:salam /app \
    && mkdir -p /app/build \
    && chmod 1777 /app/build
ENV HOME=/tmp
USER salam
ENTRYPOINT ["sh", "tools/bash/docker-dev.sh"]

FROM toolchain AS builder
COPY . .
RUN make OUTDIR=. -j"$(nproc)" && make OUTDIR=. install PREFIX=/usr/local
RUN salam version

FROM toolchain AS prod
COPY --from=builder /usr/local/bin/salam /usr/local/bin/salam
COPY --from=builder /usr/local/share/salam /usr/local/share/salam

RUN mkdir -p /work && chown -R salam:salam /work
ENV HOME=/tmp
WORKDIR /work
USER salam

HEALTHCHECK --interval=5m --timeout=10s --start-period=5s --retries=3 \
    CMD salam version || exit 1

ENTRYPOINT ["salam"]
CMD ["help"]
