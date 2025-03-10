FROM ubuntu:latest

RUN apt update \
	&& apt install -y software-properties-common \
	&& apt install -y cmake git build-essential libssl-dev

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

# emp-ag2pc
WORKDIR /root
RUN git clone https://github.com/emp-toolkit/emp-ag2pc.git
WORKDIR /root/emp-ag2pc
RUN cmake . \
	&& make \
	&& make install

WORKDIR /root