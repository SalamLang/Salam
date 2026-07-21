# syntax=docker/dockerfile:1
# Salam sandbox execution image for the runner API (Cloudflare Containers).

FROM debian:bookworm-slim AS fetch
ARG SALAM_VERSION
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl unzip \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /fetch
RUN set -eu; \
    if [ -n "${SALAM_VERSION:-}" ]; then \
      TAG="v${SALAM_VERSION}"; \
    else \
      TAG=$(curl -fsSL -o /dev/null -w '%{url_effective}' \
              "https://github.com/SalamLang/Salam/releases/latest"); \
      TAG="${TAG##*/}"; \
    fi; \
    VERSION="${TAG#v}"; \
    echo "resolved Salam release: ${TAG}"; \
    curl -fL --retry 3 --retry-delay 2 -o salam.zip \
      "https://github.com/SalamLang/Salam/releases/download/${TAG}/salam-${VERSION}-linux.zip" \
    && unzip -q salam.zip \
    && mv salam-linux /opt/salam \
    && rm -f /opt/salam/RUN-ME.txt salam.zip \
    && chmod +x /opt/salam/salam \
    && echo -n "${VERSION}" > /opt/salam/VERSION

FROM golang:1.23-bookworm AS build
WORKDIR /src
COPY server/go.mod ./
COPY server/*.go ./
COPY --from=fetch /opt/salam/VERSION /tmp/salam-version
RUN CGO_ENABLED=0 go build -trimpath -ldflags "-s -w -X main.salamVersion=$(cat /tmp/salam-version)" \
      -o /out/sandbox-server .

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
