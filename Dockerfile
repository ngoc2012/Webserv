# Use Alpine Linux as the base image
FROM alpine:latest

# Install g++ and other necessary packages
RUN apk update && apk add --no-cache g++ make

# Set the working directory
WORKDIR /app

RUN cat Makefile
RUN make re
RUN make clean

# Set the entry point to run the compiled program
# ENTRYPOINT ["./webserv"]