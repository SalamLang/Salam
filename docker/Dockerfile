FROM alpine:latest AS builder

ENV LANG=C.UTF-8 \
	LC_ALL=C.UTF-8

RUN apk add --no-cache \
	gcc \
	libc-dev \
	make

WORKDIR /build

COPY . .

RUN make

FROM alpine:latest

ENV LANG=C.UTF-8 \
	LC_ALL=C.UTF-8

WORKDIR /app

COPY --from=builder /build/salam /app/salam

CMD ["./salam"]
