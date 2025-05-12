FROM ubuntu:latest AS buildenv

RUN apt update \
	&& apt install -y software-properties-common \
	&& apt install -y cmake git build-essential libssl-dev \
	&& apt install -y gdb

# emp-tool
WORKDIR /root
RUN git clone https://github.com/emp-toolkit/emp-tool.git
WORKDIR /root/emp-tool
RUN cmake . \
	&& make \ 
	&& make install

# emp-ot
WORKDIR /root
RUN git clone https://github.com/emp-toolkit/emp-ot.git
WORKDIR /root/emp-ot
RUN cmake . \
	&& make \
	&& make install

# emp-sh2pc
WORKDIR /root
RUN git clone https://github.com/emp-toolkit/emp-sh2pc.git
WORKDIR /root/emp-sh2pc
RUN cmake . \
	&& make \
	&& make install


# Creat an image with the emp-toolkit installed and the source code of the application
FROM ubuntu:latest AS emp

RUN apt update \
	&& apt install -y software-properties-common \
	&& apt install -y cmake git build-essential libssl-dev \
	&& rm -rf /var/lib/apt/lists/*

# Copy build dependencies from other docker images
COPY --from=buildenv /usr/local/. /usr/local/.

WORKDIR /usr/src/

# Copy source code and install
COPY . /emp
RUN mkdir /emp/build
WORKDIR /emp/build
RUN cmake .. && make
WORKDIR /emp