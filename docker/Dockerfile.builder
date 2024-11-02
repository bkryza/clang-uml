FROM ubuntu:24.04
LABEL org.opencontainers.image.authors="bkryza@gmail.com"

ENV DEBIAN_FRONTEND noninteractive

#
# Install basic software
#
RUN apt update && \
    apt -y install wget software-properties-common git make grcov ninja-build \
           pkg-config gcc g++ ccache cmake libyaml-cpp-dev gnustep-make plantuml \
           gnustep-back-common libgnustep-base-dev nodejs npm

#
# Install LLVM 18 & 19
#
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    add-apt-repository "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-17 main" && \
    add-apt-repository "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-18 main" && \
    add-apt-repository "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-19 main" && \
    apt update && \
    apt -y install llvm-17 clang-17 libclang-17-dev libclang-cpp17-dev clang-tools-17 clang-format-17 clang-tidy-17 \
                   llvm-18 clang-18 libclang-18-dev libclang-cpp18-dev clang-tools-18 clang-format-18 clang-tidy-18 \
                   llvm-19 clang-19 libclang-19-dev libclang-cpp19-dev clang-tools-19 clang-format-19 clang-tidy-19 \
                   lcov zlib1g-dev libunwind-dev libdw-dev
#
# Install libobjc2
#
RUN git clone https://github.com/gnustep/libobjc2 && \
    cd libobjc2 && \
    CC=/usr/bin/clang-18 CXX=/usr/bin/clang++-18 cmake -B_build -S. && \
    cd _build && \
    make -j12 && \
    make install && \
    cd ../.. && \
    rm -rf libobjc2

#
# Install CUDA
#
RUN wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2404/x86_64/cuda-keyring_1.1-1_all.deb && \
    dpkg -i cuda-keyring_1.1-1_all.deb && \
    apt-get update && \
    apt-get -y install cuda-toolkit && \
    rm cuda-keyring_1.1-1_all.deb

RUN apt -y install nvidia-cuda-toolkit

#
# Update PlantUML plantuml.jar to 2024.7
#
RUN wget https://github.com/plantuml/plantuml/releases/download/v1.2024.7/plantuml.jar -O /usr/share/plantuml/plantuml.jar

#
# Install mermaidjs-cli
#
RUN apt -y install  && \
    npm install -g @mermaid-js/mermaid-cli@11.3.0

ENV PUPPETEER_SKIP_CHROMIUM_DOWNLOAD true

ADD puppeteer-config.json  /etc/puppeteer-config.json

RUN apt-get update && apt-get install gnupg wget -y && \
  wget --quiet --output-document=- https://dl-ssl.google.com/linux/linux_signing_key.pub | gpg --dearmor > /etc/apt/trusted.gpg.d/google-archive.gpg && \
  sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list' && \
  apt-get update && \
  apt-get install google-chrome-stable -y --no-install-recommends && \
  rm -rf /var/lib/apt/lists/* && \
  rm /etc/apt/sources.list.d/google-chrome.list

#
# Install Python3 dependencies
#
RUN apt update && \
    apt -y install python3-pip python3-full python3-jinja2 python3-lxml python3-yaml

#
# Clean up
#
RUN apt clean

ENV CCACHE_DIR /ccache
ENV CCACHE_UMASK 000
RUN mkdir ${CCACHE_DIR} && \
    chown ubuntu:ubuntu ${CCACHE_DIR}

USER ubuntu


