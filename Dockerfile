FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y --no-install-recommends gcc mono-mcs electric-fence && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN chmod u+x b.sh

CMD ["/bin/bash", "./b.sh"]
