# Dockerfile that installs pypy for maximum possible performance at compile time
FROM python:3.10.3-bullseye

# bc is only used to provide precise time in the compile script
RUN apt-get update && apt-get install -y --no-install-recommends \
		curl \
		openssl \
		xz-utils \
                automake \
                build-essential \
                clang-11 \
		cmake \
                git \
                libboost-dev \
                libboost-thread-dev \
                libclang-dev \
                libgmp-dev \
                libntl-dev \
                libsodium-dev \
                libssl-dev \
                libtool \
                vim \
                gdb \
                valgrind \
                bc \
                wget \
                zlib1g-dev \
                libbz2-dev \
                libreadline-dev \
                libsqlite3-dev \
                llvm \
                libncurses5-dev \
                libncursesw5-dev \
                tk-dev \
                libffi-dev \
        && rm -rf /var/lib/apt/lists/*

# Download and extract PyPy
RUN wget https://downloads.python.org/pypy/pypy3.11-v7.3.19-linux64.tar.bz2 && \
    tar -xvjf pypy3.11-v7.3.19-linux64.tar.bz2 && \
    mv pypy3.11-v7.3.19-linux64 /opt/pypy && \
    ln -s /opt/pypy/bin/pypy3 /usr/local/bin/pypy3

# pip for PyPy
RUN /opt/pypy/bin/pypy3 -m ensurepip && \
    /opt/pypy/bin/pypy3 -m pip install --upgrade pip ipython

RUN pip install scikit_learn