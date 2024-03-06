FROM ubuntu:noble

RUN apt-get update && \
    apt-get -y install gcc mono-mcs && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN chmod u+x b.sh

CMD ["/bin/bash", "./b.sh"]
