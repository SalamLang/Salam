FROM arm32v7/debian:sid

LABEL org.opencontainers.image.title="Salam armhf builder"
LABEL org.opencontainers.image.description="arm32v7 toolchain for building Salam with embedded LLVM+LLD"

# hadolint ignore=DL3008
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential ca-certificates ccache file git \
        llvm-22-dev liblld-22-dev libpolly-22-dev libzstd-dev zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

RUN useradd --create-home --shell /bin/bash --uid 1001 builder

ENV HOME=/tmp

HEALTHCHECK NONE

WORKDIR /work
USER builder
