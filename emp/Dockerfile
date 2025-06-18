FROM ubuntu:latest AS builder

RUN apt update && apt install -y \
		software-properties-common \
		cmake \
		git \
		build-essential \
		libssl-dev \
		python3 \
		python3-pip \
		python3-venv \
	&& rm -rf /var/lib/apt/lists/*

COPY ../requirements.txt /tmp/requirements.txt
RUN python3 -m venv /opt/venv
RUN /opt/venv/bin/pip3 install --no-cache-dir --upgrade pip
RUN /opt/venv/bin/pip3 install --no-cache-dir -r /tmp/requirements.txt

# emp-tool
WORKDIR /root
RUN git clone https://github.com/emp-toolkit/emp-tool.git
WORKDIR /root/emp-tool
RUN cmake . && make && make install

# emp-ot
WORKDIR /root
RUN git clone https://github.com/emp-toolkit/emp-ot.git
WORKDIR /root/emp-ot
RUN cmake . && make && make install

# emp-sh2pc
WORKDIR /root
RUN git clone https://github.com/emp-toolkit/emp-sh2pc.git
WORKDIR /root/emp-sh2pc
RUN cmake . && make && make install


FROM ubuntu:latest AS emp

RUN apt update && apt install -y \
		cmake \
		build-essential \
		libssl-dev \
		python3 \
		python3-pip \
	&& rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local/. /usr/local/
COPY --from=builder /opt/venv /opt/venv
RUN echo "source /opt/venv/bin/activate" >> ~/.bashrc

WORKDIR /usr/src/emp
COPY . /usr/src/emp
RUN mkdir build && cd build && cmake .. && make