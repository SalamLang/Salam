FROM debian:trixie-slim AS build
ARG SALAM_VERSION=0.5.7
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates curl gcc libc6-dev \
 && rm -rf /var/lib/apt/lists/*
RUN curl -fsSL "https://github.com/SalamLang/Salam/releases/download/v${SALAM_VERSION}/salam-${SALAM_VERSION}-linux-x86_64.tar.gz" | tar -xz -C /opt
WORKDIR /src
COPY std /srv/salam/std
COPY packages /srv/salam/packages
COPY tools/mcp/*.salam ./
ENV SALAM_STD=/srv/salam/std
RUN mkdir -p /out && /opt/salam-linux-x86_64/salam build main.salam --output=/out/salam-mcp --log-level=error

FROM debian:trixie-slim
RUN useradd --system --uid 10001 --no-create-home salam
COPY --from=build /opt/salam-linux-x86_64/salam /srv/salam/salam
COPY --from=build /srv/salam/std /srv/salam/std
COPY --from=build /srv/salam/packages /srv/salam/packages
COPY --from=build /out/salam-mcp /usr/local/bin/salam-mcp
COPY docs/ai /srv/salam/docs/ai
COPY SKILL.md /srv/salam/SKILL.md
COPY compiler/langpack.salam /srv/salam/compiler/langpack.salam
COPY tests /srv/salam/tests
USER salam
WORKDIR /tmp
ENV SALAM_MCP_ROOT=/srv/salam \
    SALAM_MCP_BIN=/srv/salam/salam \
    SALAM_STD=/srv/salam/std \
    SALAM_MCP_HOST=0.0.0.0 \
    SALAM_MCP_PORT=8080 \
    SALAM_MCP_JOBS=4 \
    SALAM_MCP_TIMEOUT=30
EXPOSE 8080
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 CMD ["/usr/local/bin/salam-mcp", "--health"]
CMD ["/usr/local/bin/salam-mcp", "--http"]
