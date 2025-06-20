FROM ubuntu:latest

RUN apt update && apt install -y \
		cmake \
		build-essential \
		libssl-dev \
		python3 \
		python3-pip \
		python3-venv \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/emp
COPY . /usr/src/emp

# Set up python dependencies for generating input data (need to use venv)
RUN python3 -m venv /opt/venv
RUN /opt/venv/bin/pip3 install --no-cache-dir --upgrade pip
RUN /opt/venv/bin/pip3 install --no-cache-dir -r requirements.txt
RUN echo "source /opt/venv/bin/activate" >> ~/.bashrc

# Build the test programs
RUN mkdir build && cd build && cmake .. && make