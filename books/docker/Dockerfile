# Salam books - one common XeLaTeX image that builds BOTH the English and the
# Persian tutorial book.

FROM texlive/texlive:latest

LABEL org.opencontainers.image.title="Salam tutorial books (XeLaTeX)"
LABEL org.opencontainers.image.description="XeLaTeX + xepersian toolchain that builds the English and Persian Salam books"

COPY build.sh /usr/local/bin/build-books
RUN chmod +x /usr/local/bin/build-books

WORKDIR /books
ENTRYPOINT ["build-books"]
CMD ["all"]
