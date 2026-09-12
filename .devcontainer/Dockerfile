FROM ubuntu:25.10

ENV VITASDK=/usr/local/vitasdk

RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    git-core \
    cmake \
    wget \
    python3 \
    ca-certificates \
    xz-utils \
    bzip2 \
    curl \
    sudo

RUN git clone https://github.com/vitasdk/vdpm /tmp/vdpm \
    && cd /tmp/vdpm \
    && ./bootstrap-vitasdk.sh \
    && chmod +x "$VITASDK/bin/pacman" \
    && ./install-all.sh \
    && rm -rf /tmp/vdpm

ENV PATH=$VITASDK/bin:$PATH
