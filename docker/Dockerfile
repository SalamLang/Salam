FROM alpine:latest

ENV LANG=C.UTF-8 \
	LC_ALL=C.UTF-8

RUN apk add --no-cache \
	gcc \
	libc-dev \
	make

WORKDIR /app

COPY . .

RUN make

RUN make run
