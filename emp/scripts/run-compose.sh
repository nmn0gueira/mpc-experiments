#!/bin/bash

set -e

# Only ask for container name if it is Bob running this script
# Get the IP address of the container by its name
CONTAINER_NAME=$1
PORT=$2
NUMBER=$3

# Resolve the IP address
CONTAINER_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_NAME)

# Run the C++ program with the resolved IP
./millionaire BOB $PORT $CONTAINER_IP $NUMBER