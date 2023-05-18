# Goofy Googlers

## Alec Goedinghaus, Brian Nguyen, Cole Strain, Dominick Won

---

## Table of Contents

- [Layout](#layout)
    - [`server.h`](#serverh)
    - [`session.h`](#sessionh)
    - [`request_handler.h`](#request_handlerh)
    - [`static_request_handler.h`](#static_request_handlerh)
    - [`echo_request_handler.h`](#echo_request_handlerh)
    - [`config_parser.h`](#config_parserh)
    - [`logging.h`](#loggingh)
- [Building](#building)
    - [Config File](#config-file)
        - [Port](#port)
        - [Request Handlers](#request-handlers)
    - [For Production](#for-production)
    - [For Testing](#for-testing)
        - [Unit Tests](#unit-tests)
        - [Integration Tests](#integration-tests)
    - [Executing](#executing)
- [Contributing](#contributing)

## Layout

### `server.h`

This is the main entry point from `main` into all of the server functionality.
It links together the port and config with Boost IO, and initializes a session.

### `session.h`

The Session class handles client connections and processes HTTP requests. It reads data from the socket, parses the request string, and determines the appropriate request handler based on configured routes. It generates a response string and writes it back to the client. The server configuration file is parsed to extract URL prefixes and handler types, which are used to create request handler factories. The Session class is responsible for coordinating the request handling process.

### `request_handler.h`

The RequestHandler class is an abstract class that provides a base implementation for handling HTTP requests. It defines common methods and properties for request handling.

Key Features:

	Parses HTTP requests and generates responses.
	Provides utility methods for token extraction, URL retrieval, and substring checks.
	Supports conversion between request/response objects and string representations.
	Includes a Status struct to represent request status.

To implement custom request handlers, include the RequestHandler header file and derive your classes from it.

### `static_request_handler.h`

This class inherits from `RequestHandler`, and overwrites the `ParseRequest` method to search for the requested static file, and return it if it exists.

### `echo_request_handler.h`

This class also inherits from `RequestHandler`, and overwrites the `ParseRequest` method to just echo back the incoming request.

### `config_parser.h`

This header mostly consists of the skeleton `NginxConfig` parsing methods we were provided with in assignment 1.

### `logging.h`

The logger is only instantiated once in `server_main.cc`.
Afterwards, whenever the logger is needed, a pointer to the `Logger` can be created and fetch the instantiated `Logger` object with `Logger::GetLogger()`.

## Building

This section covers how to set up, build, run, and test the server.

### Config File

The server config file is located in the `configuration/` directory.
The one we currently use is `starter.conf`, although any file can be specified when the server is executed, as long as the formatting is correct.
The config only needs to specify two things: a port, and a series of request handlers.

Note: none of these statements should be specified in a subconfig unless explicitly stated, i.e. do not start the config with `server {...}` or anything similar.

#### Port

The port is specified by the `port` keyword, followed by the desired port number.

#### Request Handlers

Each request handlers is specified with the `location` keyword, followed by the URL path, then the handler type, and finally a subconfig containing more data if applicable.

For static handlers, the subconfig must specify a local directory with files to serve, specified with the `root` keyword, followed by said path, see the example below:

```
location static_url StaticHandler {
  root ../static_files/static_base_directory_2;
}
```

Note that we start with the parent directory since we intend for the server to be run inside `build/`.

Echo and 404 handlers do not need any information in their subconfigs.

### For Production

We want to build this using an [out of source build](https://www.cs130.org/guides/cmake/#out-of-source-builds), so using the same commands as in the linked guide, in the project's root directory:

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Alternatively, `make` can be ran with the `-j N` flag to enable a multithreaded build, where `N` is the desired number of threads (typically 2 or 4 to match the number of cores your docker container uses).

After changes are made, the server can be rebuilt by simply running the last `make` command.

### For Testing

#### Unit Tests

After building from the above commands, the unit tests listed in `CMakeLists.txt` can be ran via:

```bash
$ make test
```

To check code coverage, we again use an out of source build.
In the project's root directory, run:

```bash
$ mkdir build_coverage
$ cd build_coverage
$ cmake -DCMAKE_BUILD_TYPE=Coverage ..
$ make coverage
```

This will run all of the unit tests and generate a coverage report found in `build_coverage/report/index.html`.

#### Integration Tests

The intergration test is simply a bash script found in `tests/integration_tests.sh`.
New tests can simply be added as a new function.

### Executing

For the paths to work properly, be sure to execute the server in the `build/` directory.
The server can then be started with:

```bash
$ ./bin/server ../configuration/starter.conf
```

or, to run the process in the background,

```bash
$ ./bin/server ../configuration/starter.conf &
```

## Contributing

Thank you for considering contributing to the GoofyGooglerServer project! We welcome contributions from the community and appreciate your efforts in making this project better.

To contribute, please follow these guidelines:

Fork the repository and create a new branch for your contribution.
Ensure that your code follows the existing coding style and conventions.
Write clear and concise commit messages explaining the purpose of your changes.
Test your changes thoroughly to ensure they do not introduce any regressions.
Update the documentation if necessary, including code comments and README files.
Submit a pull request, describing the changes you have made and the problem they solve.

If you have any questions or need assistance with your contribution, feel free to reach out to us.

We appreciate your contribution and look forward to your involvement in improving the GoofyGooglerServer project!











