# syntax=docker/dockerfile:1
#
# Salam sandbox execution image for the runner API (Cloudflare Containers).
#
# Bundles a pinned release of the Salam toolchain (binary + std/) plus a
# small static Go HTTP server (server/main.go) that runs untrusted Salam
# source under a hard wall-clock timeout, output byte caps, and a fresh
# per-request temp directory.
#
# SALAM_VERSION is bumped automatically by
# .github/workflows/runner-container-build.yml whenever a new Salam release
# is published, so this image always tracks the latest release without any
# runtime (per-request) fetch from GitHub.
#
# Hardening notes:
#  - No shell and no package manager in the final image. Salam's interpreter
#    FFI allow-list includes system()/popen(), which exec /bin/sh -c under
#    the hood - removing the shell turns that into a hard ENOENT instead of
#    a command-execution escape hatch.
#  - Runs as a dedicated non-root, no-login user.
#  - No outbound network tooling (curl/wget/dns config) present at runtime;
#    the fetch stage that needs network is discarded from the final image.
#  - If startup ever fails with "error while loading shared libraries",
#    run `ldd /opt/salam/salam` in a debug build of this image to find the
#    missing runtime lib and add the corresponding (non -dev) apt package
#    to the final stage below.

ARG SALAM_VERSION=0.2.4

# --- fetch the pinned Salam release (binary + stdlib) ---
FROM debian:bookworm-slim AS fetch
ARG SALAM_VERSION
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl unzip \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /fetch
RUN curl -fL --retry 3 --retry-delay 2 -o salam.zip \
      "https://github.com/SalamLang/Salam/releases/download/v${SALAM_VERSION}/salam-${SALAM_VERSION}-linux.zip" \
    && unzip -q salam.zip \
    && mv salam-linux /opt/salam \
    && rm -f /opt/salam/RUN-ME.txt salam.zip \
    && chmod +x /opt/salam/salam

# --- build the sandbox HTTP server (Go stdlib only, no external deps) ---
FROM golang:1.23-bookworm AS build
WORKDIR /src
COPY server/go.mod ./
COPY server/main.go ./
ARG SALAM_VERSION
RUN CGO_ENABLED=0 go build -trimpath -ldflags "-s -w -X main.salamVersion=${SALAM_VERSION}" \
      -o /out/sandbox-server .

# --- final runtime image ---
FROM debian:bookworm-slim
RUN apt-get update \
    && apt-get install -y --no-install-recommends libstdc++6 libtinfo6 libxml2 libzstd1 zlib1g \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd --gid 10001 sandbox \
    && useradd --uid 10001 --gid sandbox --no-create-home --shell /usr/sbin/nologin sandbox \
    && mkdir -p /work && chown sandbox:sandbox /work \
    && rm -f /bin/sh /bin/dash /bin/bash /usr/bin/env \
             /usr/bin/apt* /usr/bin/dpkg* /bin/mount /bin/umount 2>/dev/null || true

COPY --from=fetch /opt/salam /opt/salam
COPY --from=build /out/sandbox-server /opt/salam/sandbox-server

ENV SALAM_STD=/opt/salam/std
USER sandbox
WORKDIR /work
EXPOSE 8080
ENTRYPOINT ["/opt/salam/sandbox-server"]
