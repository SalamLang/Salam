FROM alpine:latest

ENV LANG=C.UTF-8 \
	LC_ALL=C.UTF-8

RUN apk add --no-cache \
	gcc \
	libc-dev \
	make \
	curl

WORKDIR /app

COPY ../ ./

RUN make run
