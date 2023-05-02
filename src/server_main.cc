//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include "config_parser.h"
#include "logging.h"
#include "server.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  Logger *log = Logger::get_logger();
  try {
    if (argc != 2) {
      // The server should take a path to the config file on the command line such as:
      // bin/server starter_config
      log->log_fatal("Invalid arguments. Usage: ./path/to/server path/to/config/file");
      return 1;
    }

    NginxConfigParser config_parser;
    NginxConfig config;
    config_parser.Parse(argv[1], &config);
    int server_port = config.GetPort();

    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    // For testing-- via commandline-- can also use:
    // server s(io_service, atoi(argv[1]));
    server s(io_service, server_port);

    io_service.run();

  } catch (std::exception& e) {
    std::string error = e.what();
    log->log_error("Exception: " + error);
  }

  delete log;

  return 0;
}
