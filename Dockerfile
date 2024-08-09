FROM ubuntu:latest

RUN apt-get update
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    gcc \
    mono-mcs \
    electric-fence \
    build-essential \
    libssl-dev \
    python3-pip \
    python3-venv \
    git \
    cmake \
    curl

RUN rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/emscripten-core/emsdk.git /opt/emsdk && \
    cd /opt/emsdk && \
    ./emsdk install latest && \
    ./emsdk activate latest && \
    echo "source /opt/emsdk/emsdk_env.sh" >> /etc/profile

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

RUN /opt/venv/bin/pip install pre-commit

WORKDIR /app

COPY . .

ENV TERM=xterm

RUN chmod u+x b.sh

CMD ["/bin/bash", "./b.sh"]
