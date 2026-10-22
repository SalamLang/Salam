FROM debian:trixie-slim AS build
ARG SALAM_VERSION=0.5.7
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates curl gcc libc6-dev \
 && rm -rf /var/lib/apt/lists/*
RUN curl -fsSL "https://github.com/SalamLang/Salam/releases/download/v${SALAM_VERSION}/salam-${SALAM_VERSION}-linux-x86_64.tar.gz" | tar -xz -C /opt
WORKDIR /src
COPY std /srv/salam/std
COPY packages /srv/salam/packages
COPY ai/backend/*.salam ./
ENV SALAM_STD=/srv/salam/std
RUN mkdir -p /out && /opt/salam-linux-x86_64/salam build main.salam --output=/out/salam-ai

FROM debian:trixie-slim
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --system --uid 10001 --no-create-home salam
COPY --from=build /opt/salam-linux-x86_64/salam /srv/salam/salam
COPY --from=build /srv/salam/std /srv/salam/std
COPY --from=build /srv/salam/packages /srv/salam/packages
COPY --from=build /out/salam-ai /usr/local/bin/salam-ai
USER salam
ENV AI_PORT=8080 AI_HOST=0.0.0.0 AI_SALAM_BIN=/srv/salam/salam SALAM_STD=/srv/salam/std
EXPOSE 8080
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 CMD ["/usr/local/bin/salam-ai", "--health"]
CMD ["/usr/local/bin/salam-ai"]
