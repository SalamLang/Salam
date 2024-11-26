FROM debian:bullseye-slim

ENV DEBIAN_FRONTEND=noninteractive \
	LANG=C.UTF-8 \
	LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
	build-essential \
	gcc \
	curl \
	ca-certificates \
	make \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /

COPY . .

RUN make install && make run

CMD ["./salam"]
