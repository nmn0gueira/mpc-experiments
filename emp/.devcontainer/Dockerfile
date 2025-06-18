FROM ubuntu:latest

RUN apt update && apt install -y \
		software-properties-common \
		cmake \
		git \
		build-essential \
		libssl-dev \
		gdb \
		python3 \
		python3-pip \
		python3-venv \
	&& rm -rf /var/lib/apt/lists/*

# Install python dependencies for generating input data (need to use venv)
COPY ../requirements.txt /tmp/requirements.txt
RUN python3 -m venv /opt/venv
RUN /opt/venv/bin/pip3 install --no-cache-dir --upgrade pip
RUN /opt/venv/bin/pip3 install --no-cache-dir -r /tmp/requirements.txt
RUN echo "source /opt/venv/bin/activate" >> ~/.bashrc

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