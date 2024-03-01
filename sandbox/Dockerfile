FROM debian:trixie-slim AS build
ARG SALAM_VERSION=0.5.9
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates curl gcc libc6-dev \
 && rm -rf /var/lib/apt/lists/*
RUN curl -fsSL "https://github.com/SalamLang/Salam/releases/download/v${SALAM_VERSION}/salam-${SALAM_VERSION}-linux-x86_64.tar.gz" | tar -xz -C /opt
WORKDIR /src
COPY std /srv/salam/std
COPY sandbox/main.salam ./
ENV SALAM_STD=/srv/salam/std
RUN mkdir -p /out && /opt/salam-linux-x86_64/salam build main.salam --output=/out/salam-sandbox

FROM debian:trixie-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates docker-cli coreutils \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --system --uid 10001 --no-create-home salam
COPY --from=build /out/salam-sandbox /usr/local/bin/salam-sandbox
USER salam
ENV SANDBOX_PORT=8090
EXPOSE 8090
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 CMD ["/usr/local/bin/salam-sandbox", "--health"]
CMD ["/usr/local/bin/salam-sandbox"]
