# Use Alpine Linux as the base image
FROM alpine:latest

# Install g++ and other necessary packages
RUN apk update && \
    apk add --no-cache g++ make && \
    rm -rf /var/cache/apk/*

# Set the working directory
WORKDIR /app

COPY ./build.sh .

RUN chmod +x build.sh

# Set the entry point to run the compiled program
ENTRYPOINT ["/bin/sh", "-c", "chmod +x /app/build.sh && /app/build.sh"]