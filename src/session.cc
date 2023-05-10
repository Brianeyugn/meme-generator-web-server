#include "session.h"

#include <cstring>  // For strcpy
#include <fstream>
#include <iostream> // For Debugging
#include <sstream>  // For iostringstream

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "logging.h"
#include "echo_request_handler.h"
#include "request_handler.h"
#include "static_request_handler.h"

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
    // Feed handlers to handler vector.
    std::vector<RequestHandler*> handlers;
    // Parse config file and handle configs dynamically
    ParseConfigFile(handlers);

    // (DEBUG) Dump client request to Server console.
    log->LogDebug("Client requested: " + request_string);

    // Give request_string to handlers to produce response_string.
    std::string response_string = HandleRequest(request_string, handlers);

    // (DEBUG) Dump response to Server console.
    // cout << "8##" << response_string << "$$$" << endl;

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
    for (int i = 0; i < handlers.size(); i++) {
      delete handlers[i];
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
std::string Session::HandleRequest(std::string request_string, std::vector<RequestHandler*> handlers) {
  Logger *log = Logger::GetLogger();
  log->LogDebug("Handling request");

	std::string response_string;
	RequestHandler* rh = new RequestHandler("", ""); // Default request handler (Always returns 404 NOT FOUND).

	bool found_matching_handler = false;
	for (int i = 0; i < handlers.size(); i++) {
		handlers[i]->SetRequestString(request_string);
		if (handlers[i]->IsMatchingHandler()) {
      log->LogDebug("Found matching handler");
			found_matching_handler = true;
			handlers[i]->ParseRequest(); // Polymorphic call to obtain response_string.
			response_string = handlers[i]->GetResponseString_();
		}
	}

	if (!found_matching_handler) { // Let default handler handle if no matching handler found.
    log->LogDebug("No matching handler found, using default");
		rh->SetRequestString(request_string);
		rh->ParseRequest();
		response_string = rh->GetResponseString_();
	}

	// Clean up memory.
	delete rh;

	return response_string;
}

void Session::ParseConfigFile(std::vector<RequestHandler*>& handlers) {
  Logger *log = Logger::GetLogger();

  for (auto statement : config_.statements_) {
    if (statement.get()->tokens_[0] != "location") {
      continue;
    }

    std::string handler_type, url_prefix, directory_path;
    handler_type = statement.get()->tokens_[1];
    url_prefix = statement.get()->tokens_[2];
    log->LogDebug("handler type: " + handler_type);
    log->LogDebug("url prefix: " + url_prefix);

    // Create handler based on type.
    if (handler_type == "static") {
      for (auto child_statement : statement->child_block_.get()->statements_) {
        if (child_statement.get()->tokens_[0] != "root") {
          continue;
        }

        directory_path = child_statement.get()->tokens_[1];
      }

      log->LogDebug("directory path: " + directory_path);
      StaticRequestHandler* srh = new StaticRequestHandler("", url_prefix, directory_path);
      handlers.push_back(srh);
    } else if (handler_type == "echo") {
      EchoRequestHandler* erh = new EchoRequestHandler("", url_prefix);
      handlers.push_back(erh);
    } else {
      // Invalid handler type specified in config file.
      log->LogWarn("Invalid handler type specified in config file: " + handler_type);
    }
  }
}

void Session::ParseConfigFile(const std::string& filename, std::vector<RequestHandler*>& handlers) {
  Logger *log = Logger::GetLogger();
  std::ifstream config_file(filename);
  std::string line;

  while (getline(config_file, line)) {
    // Parse line to extract handler type, URL prefix, and directory path.
    // For example: "static /static1 ../static_files/static_base_directory_1"
    std::istringstream iss(line);
    std::string handler_type, url_prefix, directory_path;
    iss >> handler_type >> url_prefix >> directory_path;
    log->LogDebug("handler type: " + handler_type);
    log->LogDebug("url prefix: " + url_prefix);
    log->LogDebug("directory path: " + directory_path);

    // Create handler based on type.
    if (handler_type == "static") {
      StaticRequestHandler* srh = new StaticRequestHandler("", url_prefix, directory_path);
      handlers.push_back(srh);
    } else if (handler_type == "echo") {
      EchoRequestHandler* erh = new EchoRequestHandler("", url_prefix);
      handlers.push_back(erh);
    } else {
      // Invalid handler type specified in config file.
      log->LogWarn("Invalid handler type specified in config file: " + handler_type);
    }
  }
}
