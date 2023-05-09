//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include "config_parser.h"
#include "logging.h"
#include "server.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  Logger *log = Logger::GetLogger();
  try {
    if (argc != 2) {
      // The Server should take a path to the config file on the command line such as:
      // bin/Server starter_config
      log->LogFatal(
          "Invalid arguments. Usage: ./path/to/Server path/to/config/file");

      return 1;
    }

    // Log various signals that could stop the server
    std::signal(SIGINT, log->SignalHandler);
    std::signal(SIGQUIT, log->SignalHandler);
    std::signal(SIGTERM, log->SignalHandler);

    NginxConfigParser config_parser;
    NginxConfig config;
    config_parser.Parse(argv[1], &config);
    int server_port = config.GetPort();

    boost::asio::io_service io_service;
    
    // For testing-- via commandline-- can also use:
    // Server s(io_service, std::atoi(argv[1]));
    Server s(io_service, server_port, config);
    io_service.run();

    log->LogInfo("Stopping Server");
  } catch (std::exception& e) {
    std::string error = e.what();
    log->LogError("Exception: " + error);
  }

  delete log;

  return 0;
}
