FROM ubuntu:22.04

WORKDIR /root
RUN apt-get update
RUN apt install -y gcc-riscv64-linux-gnu  \
    && apt install -y autoconf automake autotools-dev curl libmpc-dev libmpfr-dev libgmp-dev \
    gawk build-essential bison flex texinfo gperf libtool patchutils bc \
    zlib1g-dev libexpat-dev git    \
    && apt install -y qemu-system-misc    \
    && apt install -y gdb-multiarch   \
    && rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]