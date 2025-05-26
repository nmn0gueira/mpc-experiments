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
                nano \
                gdb \
                valgrind \
                bc \
        && rm -rf /var/lib/apt/lists/*

COPY ../requirements.txt /tmp/requirements.txt
RUN pip install --no-cache-dir --upgrade pip
RUN pip install --no-cache-dir -r /tmp/requirements.txt