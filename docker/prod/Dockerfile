ARG emp_image="nmn0gueira/emp-sh2pc:latest"

FROM ${emp_image} as emp

FROM ubuntu:latest as dev

# Required Packages ....
RUN apt-get -y update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    ccache \
    cmake \
    git \
    libc-ares-dev \
    libboost-dev \
    libboost-context-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    libboost-system-dev \
    libcurl4-openssl-dev \
    libdouble-conversion-dev \
    libevent-dev \
    libgflags-dev \
    libgmp-dev \
    libgoogle-glog-dev \
    libre2-dev \
    libssl-dev \
    zlib1g-dev

RUN mkdir -p /home/ubuntu/build/emp-proto
WORKDIR /home/ubuntu/build/emp-proto

# Copy build dependencies from other docker images
COPY --from=emp /usr/local/. /usr/local/.