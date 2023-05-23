#include "session.h"

#include <cstring>  // For strcpy
#include <fstream>
#include <iostream> // For Debugging
#include <sstream>  // For iostringstream

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/bind.hpp>

#include "config_parser.h"
#include "echo_request_handler.h"
#include "logging.h"
#include "request_handler.h"
#include "static_request_handler.h"
#include "api_request_handler.h"
#include "request_factory.h"

namespace http = boost::beast::http;
using http::string_body;
using boost::asio::ip::tcp;

Session::Session(boost::asio::io_service& io_service, const NginxConfig& config)
  : socket_(io_service), config_(config) {
  Logger *log = Logger::GetLogger();
  log->LogInfo("Initialized Session");
}

// Entryway into Session-- Session begins first read here.
void Session::Start() {
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&Session::HandleRead, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

void Session::HandleRead(const boost::system::error_code& error,
    std::size_t bytes_transferred) {
  Logger *log = Logger::GetLogger();

  if (error) {
    log->LogError("Read handler failed with error code: " + std::to_string(error.value()));
    delete this;
    return;
  }

  log->LogDebug("Handling read");
  bool request_found = false;
  std::string request_string;

  // Add read to working read_string_buffer_
  read_string_buffer_.append(std::string(data_, 0, bytes_transferred));

  // Parse read_string_buffer_ for single Request
  int pos = read_string_buffer_.find("\n\n");
  if (pos != std::string::npos) {
    request_string = read_string_buffer_.substr(0, pos + 2);
    request_found = true;
    read_string_buffer_.erase(0, request_string.length());
  } else {
    pos = read_string_buffer_.find("\r\n\r\n");
    if (pos != std::string::npos) { 
      request_string = read_string_buffer_.substr(0, pos + 4);
      request_found = true;
      read_string_buffer_.erase(0, request_string.length());
    } else {
      request_found = false;
    }
  }

  if (request_found) {
    // Parse config file and handle configs dynamically
    std::vector<ParsedConfig*> parsed_configs = ParseConfigFile();
    std::map<std::string, RequestHandlerFactory*> routes;
    CreateHandlers(parsed_configs, routes);

    // (DEBUG) Dump client request to Server console.
    log->LogDebug("Client requested: " + request_string);

    // Give request_string to handlers to produce response_string.
    std::string response_string = HandleRequest(request_string, routes);
    log->LogDebug("response string: " + response_string);
    // (DEBUG) Dump response to Server console.
    // std::cout << "8##" << response_string << "$$$" << std::endl;

    // Bool value to determine if we should shutdown connection
    bool close_request_exists = false;
    close_request_exists = RequestHandler::ContainsSubstring(response_string, "Connection: close");
    if(close_request_exists == true) {
      //Write back to client and then shutdown connection
      log->LogDebug("Writing back to client with IP: " + socket_.remote_endpoint().address().to_string());
      boost::asio::async_write(socket_,
        boost::asio::buffer(response_string),
        boost::bind(&Session::HandleWriteShutdown, this,
          boost::asio::placeholders::error));
    }
    else {
          // Write back to client.
    log->LogDebug("Writing back to client with IP: " + socket_.remote_endpoint().address().to_string());
    boost::asio::async_write(socket_,
      boost::asio::buffer(response_string),
      boost::bind(&Session::HandleWrite, this,
        boost::asio::placeholders::error));
    }
    // Clean up memory.
    for (auto& parsed_config : parsed_configs) {
      delete parsed_config;
    }

  } else { // request_found == false-- just write nothing to client so that we can read again to accumulate read_string_buffer_.
    boost::asio::async_write(socket_,
      boost::asio::buffer(std::string("")),
      boost::bind(&Session::HandleWrite, this,
        boost::asio::placeholders::error));
  }
}

void Session::HandleWrite(const boost::system::error_code& error) {
  Logger *log = Logger::GetLogger();

  if (error) {
    log->LogError("Write handler failed with error code: " + std::to_string(error.value()));
    delete this;
    return;
  } 

  log->LogDebug("Handling write");
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
    boost::bind(&Session::HandleRead, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
}

void Session::HandleWriteShutdown(const boost::system::error_code& error) {
  Logger *log = Logger::GetLogger();

  if (error) {
    log->LogError("Write handler failed with error code: " + std::to_string(error.value()));
    delete this;
    return;
  } 

  boost::system::error_code shutdown_error;
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, shutdown_error);
  if (shutdown_error) {
    log->LogError("Failed to close the socket, Write handler failed with error code: " + std::to_string(shutdown_error.value()));
  }

  boost::system::error_code close_error;
  socket_.close(close_error);
  if (error) {
    log->LogError("Failed to close the socket, Write handler failed with error code: " + std::to_string(close_error.value()));
  }

  log->LogDebug("Client connection closed");
}

// Given request string and vector of RequestHandler pointers.
// Matches string with appropriate request handler.
// Returns the response string.
std::string Session::HandleRequest(const std::string request_string, std::map<std::string, RequestHandlerFactory*>& routes) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("Handling request");
  log->LogDebug("Request: " + request_string);
  // Convert request_string to boost request object.
  http::request<string_body> boost_request = RequestHandler::StringToRequest(request_string);
  std::string request_target = boost_request.target().to_string();
  // TODO: check parsing for boost_request
  
  // extract the HTTP method from the request string to prepend to `request_target`
  size_t first_space_position = request_string.find_first_of(" ");
  std::string request_method = request_string.substr(0, first_space_position);
  std::string method_to_insert = request_method + " ";
  request_target.insert(0, method_to_insert);
  request_target += " HTTP/1.1\r\n";
  log->LogDebug("Request target: " + request_target);

  // Boost response object.
  http::response<string_body> boost_response;

  std::string matching_location = "";
  std::size_t longest_match_length = 0;

  for (const auto& route : routes) {
    const std::string& location = route.first;
    log->LogDebug("route.first/location: " + location);
    log->LogDebug("request target: " + request_target);

    if (location[0] != '/')
    {
      if (RequestHandler::ContainsSubstring(request_target, location) != false && location.length() > longest_match_length) {
        longest_match_length = location.length();
        matching_location = location;
        log->LogDebug("matching location: " + matching_location);
      }
    }
  }

  // Remove the leading '/' if present in matching_location
  if (!matching_location.empty() && matching_location[0] == '/') {
    matching_location.erase(0, 1);
  }

  log->LogDebug("checking if matching route is found");
  // Check if matching route is found
  if (matching_location.size() > 1) {
    log->LogDebug("matching location: " + matching_location);

    RequestHandlerFactory* factory = routes[matching_location];
    if (factory == nullptr) {
      log->LogWarn("No handler found for URL: " + boost_request.target().to_string());
      return "";
    }
    
    if (matching_location.size() > 1) {
      matching_location.erase(0,1);
    }

    boost_request = RequestHandler::StringToRequest(request_target);
    // TODO : second parameter of create needs to work dynamically
    // TODO: aaron do something with file_to_id here
    std::map<std::string, std::vector<int>> file_to_id;
    RequestHandler* handler = factory->create(matching_location, "../static_files/static_base_directory_1", file_to_id);

    log->LogDebug("Serving request");
    // Serve the request
    handler->ParseRequest(boost_request, boost_response);

    log->LogDebug("Converting boost response object to string");
    // Convert boost response object to string
    std::string response_string = RequestHandler::ResponseToString(boost_response);

    log->LogDebug("HandleRequest finished");
    // Cleanup
    delete handler;

    return response_string;
  } else {
    // No matching route found
    log->LogWarn("No matching route found for URL: " + boost_request.target().to_string());
    // TODO: return appropriate error response
    return "";
  }
}

std::vector<ParsedConfig*> Session::ParseConfigFile() {
  Logger *log = Logger::GetLogger();
  std::vector<ParsedConfig*> parsed_configs;

  // NginxConfig subconfig = *(config_.statements_[0].get()->child_block_.get());
  for (auto statement : config_.statements_) {
    log->LogDebug("parsing statement: " + statement.get()->ToString(0));
    if (statement.get()->tokens_[0] != "location") {
      continue;
    }

    std::string handler_type, url_prefix;
    url_prefix = statement.get()->tokens_[1];
    handler_type = statement.get()->tokens_[2];
    log->LogDebug("url prefix: " + url_prefix);
    log->LogDebug("handler type: " + handler_type);

    // Set ParsedConfig struct.
    ParsedConfig* parsed_config = new ParsedConfig();
    if (handler_type == "StaticHandler") {
      parsed_config->handler_type = HandlerType::kStatic;
    } else if (handler_type == "EchoHandler") {
      parsed_config->handler_type = HandlerType::kEcho;
    } else if (handler_type == "None") {
      parsed_config->handler_type = HandlerType::kNone;
    } else if (handler_type == "ApiHandler") {
      parsed_config->handler_type = HandlerType::kApi;
    }
    parsed_config->url_prefix = url_prefix;
    parsed_config->statement = statement;

    parsed_configs.push_back(parsed_config);
  }

  return parsed_configs;
}

void Session::CreateHandlers(std::vector<ParsedConfig*>& parsed_configs, std::map<std::string, RequestHandlerFactory*>& routes) {
  Logger *log = Logger::GetLogger();
  std::string directory_path, data_path;
  // Create handler based on type.
  for (ParsedConfig* parsed_config : parsed_configs) {
    switch (parsed_config->handler_type)
    {
      case HandlerType::kStatic: {
        for (auto child_statement : parsed_config->statement->child_block_.get()->statements_) {
          if (child_statement.get()->tokens_[0] != "root") {
            continue;
          }

          directory_path = child_statement.get()->tokens_[1];
        }

        log->LogDebug("directory path: " + directory_path);
        RequestHandlerFactory* factory = new StaticRequestHandlerFactory();
        log->LogDebug("pushing static handler");
        log->LogDebug("url prefix: " + parsed_config->url_prefix);
        routes[parsed_config->url_prefix] = factory;
        break;
        }
      case HandlerType::kApi: {
        for (auto child_statement : parsed_config->statement->child_block_.get()->statements_) {
          if (child_statement.get()->tokens_[0] != "data_path") {
            continue;
          }

          data_path = child_statement.get()->tokens_[1];
        }

        log->LogDebug("data path: " + directory_path);
        RequestHandlerFactory* factory = new ApiRequestHandlerFactory();
        log->LogDebug("pushing api handler");
        log->LogDebug("url prefix: " + parsed_config->url_prefix);
        routes[parsed_config->url_prefix] = factory;
        break;
      }
      case HandlerType::kEcho: {
        RequestHandlerFactory* factory = new EchoRequestHandlerFactory();
        log->LogDebug("pushing echo handler");
        log->LogDebug("url prefix: " + parsed_config->url_prefix);
        routes[parsed_config->url_prefix] = factory;
        break;
      }
      case HandlerType::kNone: {
        // Can't instantiate abstract object
        // RequestHandlerFactory* factory = new RequestHandlerFactory();
        log->LogDebug("pushing non handler");
        routes[parsed_config->url_prefix] = nullptr;
        break;
      }
      default: {
        // Invalid handler type specified in config file.
        log->LogWarn("Invalid handler type specified in config file: " + parsed_config->handler_type);
      }
    }
  }
}
