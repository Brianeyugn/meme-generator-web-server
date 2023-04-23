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

# Generate coverage report
RUN pwd
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage
