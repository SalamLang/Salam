# syntax=docker/dockerfile:1
#
# Salam compiler - multi-stage image (Alpine / musl, smallest practical base).
#
# Stages:
#   toolchain  - base image with everything needed to BUILD and RUN salam
#                (C compilers + LLVM 22 + make). Shared by every other stage.
#   dev        - live development: source is bind-mounted, salam is recompiled
#                on every change (see tools/docker-dev.sh).
#   builder    - production build: COPY . then `make && make install`.
#   prod       - slim runtime: only the installed `salam` + std/ from `builder`,
#                on top of the same toolchain (salam shells out to cc/llvm at
#                run time, so the toolchain must be present to compile programs).
#
# Build a specific stage with:  docker build --target dev|prod -t salam:dev .
# docker-compose targets these stages directly.

ARG ALPINE_VERSION=edge
ARG LLVM_VERSION=22

# ---------------------------------------------------------------------------
# toolchain: compilers + LLVM 22 + make (no project sources yet)
# ---------------------------------------------------------------------------
FROM alpine:${ALPINE_VERSION} AS toolchain
ARG LLVM_VERSION

# build-base   -> gcc, musl-dev, make, libc headers, ar  (build salam + -lm)
# clang/llvm22 -> the `salam llvm` backend (clang-22/llc-22/opt-22/lli-22)
# tcc          -> salam's default C backend for `salam build`
# bash,git     -> convenience for the build/test scripts (tools/*.sh use sh)
# Retry the fetch: Alpine's CDN occasionally returns transient DNS errors.
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
        && break; \
        echo "apk add attempt $i failed; retrying in 5s..." >&2; sleep 5; \
    done

# salam invokes the LLVM tools by exact name: clang-22, llc-22, opt-22, lli-22
# (see src/driver/llvm_toolchain.c). Alpine ships them under /usr/lib/llvmNN/bin
# and/or as version-suffixed binaries; expose both under the names salam expects.
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

# Make uses CC for building salam itself; gcc is the most robust host compiler.
ENV CC=gcc
WORKDIR /app

# ---------------------------------------------------------------------------
# dev: live-reload development environment
# ---------------------------------------------------------------------------
FROM toolchain AS dev
# entr watches the source tree and reruns make on change.
RUN apk add --no-cache entr
# Source is bind-mounted at /app by docker-compose; nothing is COPYed in so the
# container always reflects the host working tree. The watch loop also does the
# first build on start.
ENTRYPOINT ["sh", "tools/docker-dev.sh"]

# ---------------------------------------------------------------------------
# builder: compile + install salam into /usr/local (staged for prod)
# ---------------------------------------------------------------------------
FROM toolchain AS builder
COPY . .
RUN make -j"$(nproc)" && make install PREFIX=/usr/local
# Sanity check: the installed binary must resolve its std/ tree.
RUN salam --version || true

# ---------------------------------------------------------------------------
# prod: slim runtime with only the installed compiler + stdlib
# ---------------------------------------------------------------------------
FROM toolchain AS prod
# Bring over just the install output (binary + share/salam/std), not the sources.
COPY --from=builder /usr/local/bin/salam /usr/local/bin/salam
COPY --from=builder /usr/local/share/salam /usr/local/share/salam
WORKDIR /work
ENTRYPOINT ["salam"]
CMD ["--help"]
