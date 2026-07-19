# syntax=docker/dockerfile:1
#
# Salam benchmark suite - reproducible cross-language toolchain image.
#

ARG ALPINE_VERSION=edge
ARG LLVM_VERSION=22

FROM alpine:${ALPINE_VERSION} AS toolchain
ARG LLVM_VERSION
SHELL ["/bin/ash", "-o", "pipefail", "-c"]

RUN for i in 1 2 3 4 5; do \
        apk add --no-cache \
            build-base \
            g++ \
            make \
            musl-dev \
            tcc \
            tcc-libs-static \
            clang${LLVM_VERSION} \
            llvm${LLVM_VERSION} \
            rust \
            cargo \
            go \
            python3 \
            bash \
            git \
            coreutils \
            sqlite-dev \
            sqlite-static \
            mariadb-connector-c-dev \
        && break; \
        echo "apk add attempt $i failed; retrying in 5s..." >&2; sleep 5; \
    done

RUN set -eux; \
    ok=0; \
    for i in 1 2 3 4 5; do \
        if apk add --no-cache php php-cli 2>/dev/null; then ok=1; break; fi; \
        pkg="$(apk search -e 'php*-cli' 2>/dev/null | sort -V | tail -1)"; \
        if [ -n "$pkg" ] && apk add --no-cache "$pkg" 2>/dev/null; then ok=1; break; fi; \
        echo "apk add php attempt $i failed; retrying in 5s..." >&2; sleep 5; \
    done; \
    [ "$ok" = 1 ]; \
    command -v php

RUN set -eux; \
    LLVM_BIN="/usr/lib/llvm${LLVM_VERSION}/bin"; \
    for t in clang llc opt lli; do \
        if   [ -x "$LLVM_BIN/$t" ]; then src="$LLVM_BIN/$t"; \
        elif command -v "$t-${LLVM_VERSION}" >/dev/null 2>&1; then src="$(command -v "$t-${LLVM_VERSION}")"; \
        elif command -v "$t" >/dev/null 2>&1; then src="$(command -v "$t")"; \
        else echo "missing LLVM tool: $t" >&2; exit 1; fi; \
        ln -sf "$src" "/usr/local/bin/$t-${LLVM_VERSION}"; \
    done

RUN { gcc --version; g++ --version; rustc --version; go version; \
      python3 --version; php --version; tcc -v || true; \
      clang-${LLVM_VERSION} --version; } | tee /etc/bench-toolchain-versions.txt

ARG SALAM_UID=1000
ARG SALAM_GID=1000
RUN addgroup -g "${SALAM_GID}" salam \
    && adduser -D -u "${SALAM_UID}" -G salam salam

ENV GOCACHE=/tmp/go-cache GOPATH=/tmp/go CARGO_HOME=/tmp/cargo
WORKDIR /repo

FROM toolchain AS salam-builder
COPY compiler /repo/compiler
RUN cd /repo/compiler && make -j"$(nproc)"
RUN /repo/salam version

FROM toolchain AS bench
COPY --from=salam-builder /repo/salam /repo/salam
COPY --from=salam-builder /repo/compiler/std /repo/compiler/std
COPY benchmark /repo/benchmark

ENV SALAM_BIN=/repo/salam \
    SALAM_STD=/repo/compiler/std \
    BENCH_WORK=/tmp/salam_bench_work

RUN printf '%s\n' \
    '#!/bin/sh' \
    'set -e' \
    'cd /repo/benchmark' \
    '"$SALAM_BIN" build runner.salam --output=/tmp/bench_runner' \
    'exec /tmp/bench_runner "$@"' \
    > /repo/entrypoint.sh \
    && chmod +x /repo/entrypoint.sh \
    && chown -R salam:salam /repo/benchmark

USER salam
WORKDIR /repo/benchmark
ENTRYPOINT ["/repo/entrypoint.sh"]

FROM toolchain AS shell
ENTRYPOINT ["bash"]
