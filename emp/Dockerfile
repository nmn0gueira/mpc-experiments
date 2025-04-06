

# Image built from .devcontainer/Dockerfile
ARG os_release="latest"
ARG emp_image="nmn0gueira/emp-dev:latest"

FROM ${emp_image} AS emp

FROM ubuntu:${os_release}

RUN apt update \
	&& apt install -y software-properties-common \
	&& apt install -y cmake git build-essential libssl-dev

WORKDIR /root

# Copy build dependencies from other docker images
COPY --from=emp /usr/local/. /usr/local/.

# Copy source code and install
COPY . /emp-test
WORKDIR /emp-test/build
RUN cmake .. \
	&& make

# Run the application
CMD ["/bin/bash"]