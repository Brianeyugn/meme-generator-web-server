### Build/test container ###
# Define builder stage
FROM goofy-googlers-server:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN pwd
RUN find . -name "server"
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:jammy as deploy

COPY --from=builder /usr/src/project/build/bin/server .
COPY --from=builder /usr/src/project/configuration/docker.conf .

# Copy static file content into deployment image
COPY --from=builder /usr/src/project/data ./data

# Copy meme images and database into deployment image
COPY --from=builder /usr/src/project/memes_dir ./memes_dir

# Copy tests into deployment image (including meme images for test_memes_dir)
COPY --from=builder /usr/src/project/tests ./tests

EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["docker.conf"]
