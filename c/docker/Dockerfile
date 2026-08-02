# syntax=docker/dockerfile:1
#
# Salam compiler - multi-stage image (Alpine / musl, smallest practical base).
#

ARG ALPINE_VERSION=edge
ARG LLVM_VERSION=22

FROM alpine:${ALPINE_VERSION} AS toolchain
ARG LLVM_VERSION
SHELL ["/bin/ash", "-o", "pipefail", "-c"]

# trivy:ignore:AVD-DS-0025
RUN --mount=type=cache,target=/var/cache/apk,sharing=locked \
    for i in 1 2 3 4 5; do \
        apk add -U --cache-dir=/var/cache/apk \
            build-base \
            make \
            musl-dev \
            ccache \
            tcc \
            tcc-libs-static \
            clang${LLVM_VERSION} \
            llvm${LLVM_VERSION} \
            bash \
            git \
            curl \
            sqlite-dev \
            sqlite-static \
            mariadb-connector-c-dev \
            hiredis-dev \
            openssl-dev \
            gtk+3.0-dev \
            webkit2gtk-4.1-dev \
            nodejs \
            npm \
            qemu-aarch64 \
            qemu-arm \
            qemu-i386 \
            wine \
        && break; \
        echo "apk add attempt $i failed; retrying in 5s..." >&2; sleep 5; \
        if [ "$i" = 5 ]; then \
            echo "apk add failed after 5 attempts; aborting build." >&2; \
            exit 1; \
        fi; \
    done

# Wine defaults: WINEDEBUG=-all silences its very chatty fixme:/err: debug
# channels, which otherwise land on stdout/stderr right alongside whatever
# the guest .exe prints and corrupt run-tests.sh's exact-match output
# comparison for cross/*/x86_64-w64-windows-gnu. XDG_RUNTIME_DIR must be a
# real, 0700 directory the current user owns - wineserver uses it for its
# IPC socket and refuses to work correctly without it (this was silently
# breaking every wine invocation: "wine: failed to open <exe>" even though
# the exe existed and the earlier build succeeded).
ENV WINEDEBUG=-all
ENV XDG_RUNTIME_DIR=/tmp/xdg-runtime

# Wine's first-ever run for a given $HOME (here root's, since this image
# runs as root) bootstraps a whole $WINEPREFIX (registry hives, drive_c,
# fonts, background services like explorer.exe/services.exe) - slow, and
# noisy even with WINEDEBUG=-all (mostly one-time nodrv_CreateWindow
# warnings from those background services probing for a display that
# doesn't exist in CI, which is expected/harmless for a console-only
# target). Doing that bootstrap here, at build time, means: (a) it happens
# once and gets cached like any other layer/apk package instead of on
# every test run, and (b) any real init failure shows up as a build error
# instead of a mysteriously-failing test run. `wineserver -w` waits for
# the background services wineboot spawns to fully exit before the layer
# is committed, so the prefix that ships in the image is a clean,
# quiescent one rather than one frozen mid-boot. Both waits are
# timeout-bounded ("|| true"): a background service that never settles
# (e.g. still retrying a display connection) must not be able to hang the
# whole image build - the presence of system.reg below is what actually
# decides pass/fail, not a clean wineserver exit.
RUN mkdir -p "$XDG_RUNTIME_DIR" && chmod 0700 "$XDG_RUNTIME_DIR" \
    && timeout 60 wineboot --init >/tmp/wineboot-init.log 2>&1; \
    timeout 30 wineserver -w || wineserver -k || true; \
    if [ ! -f /root/.wine/system.reg ]; then \
        echo "wine prefix init failed:" >&2; cat /tmp/wineboot-init.log >&2; exit 1; \
    fi; \
    rm -f /tmp/wineboot-init.log

RUN node -v && npm -v

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

WORKDIR /app

FROM toolchain AS dev
# trivy:ignore:AVD-DS-0025
RUN --mount=type=cache,target=/var/cache/apk,sharing=locked \
    apk add -U --cache-dir=/var/cache/apk entr
RUN chown -R salam:salam /app \
    && mkdir -p /app/build \
    && chmod 1777 /app/build
ENV HOME=/tmp
USER salam
ENTRYPOINT ["sh", "tools/bash/docker-dev.sh"]

FROM toolchain AS builder
COPY c/ .
COPY std/ ./std
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
