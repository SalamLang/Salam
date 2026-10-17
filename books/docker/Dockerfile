FROM texlive/texlive:latest@sha256:0f4884ca7d287f47ab466d2dd405d4c901c576585363f628863a7ca7eea21301

LABEL org.opencontainers.image.title="Salam tutorial books (XeLaTeX)"
LABEL org.opencontainers.image.description="XeLaTeX + xepersian toolchain that builds the English and Persian Salam books"

COPY build.sh /usr/local/bin/build-books
RUN chmod +x /usr/local/bin/build-books

RUN mkdir -p /books && chown -R texlive:texlive /books

ENV HOME=/tmp

HEALTHCHECK NONE

WORKDIR /books
USER texlive
ENTRYPOINT ["build-books"]
CMD ["all"]
