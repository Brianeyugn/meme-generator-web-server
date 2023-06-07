### Build/test container ###
# Define builder stage
FROM goofy-googlers-server:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Install SQLite3
RUN (cd /
    && curl https://www.sqlite.org/2023/sqlite-autoconf-3420000.tar.gz --output sqlite-autoconf-3420000.tar.gz
    && tar xvfz sqlite-autoconf-3420000.tar.gz)

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

# Copy SQLite3 installation files from builder and install in deployment image
COPY --from=builder /sqlite-autoconf-3420000 ./sqlite-autoconf-3420000

RUN (cd sqlite-autoconf-3420000
    && ./configure --prefix=/usr/local
    && make
    && make install)

EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["docker.conf"]
