FROM debian:trixie-slim AS salam
ARG SALAM_VERSION=0.5.9
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates curl \
 && rm -rf /var/lib/apt/lists/*
RUN curl -fsSL "https://github.com/SalamLang/Salam/releases/download/v${SALAM_VERSION}/salam-${SALAM_VERSION}-linux-x86_64.tar.gz" | tar -xz -C /opt

FROM debian:trixie-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates curl git openssh-client build-essential pkg-config jq ripgrep python3 python3-pip python3-venv nodejs npm unzip zip less procps file \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --uid 1000 --create-home --home-dir /home/sandbox --shell /bin/bash sandbox
COPY --from=salam /opt/salam-linux-x86_64 /opt/salam
ENV PATH=/opt/salam:/work/.home/.local/bin:$PATH SALAM_STD=/opt/salam/std
USER 1000:1000
WORKDIR /work
