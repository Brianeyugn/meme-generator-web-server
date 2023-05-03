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
using namespace std;

session::session(boost::asio::io_service& io_service)
  : socket_(io_service) {
  Logger *log = Logger::get_logger();
  log->log_info("Initialized session");
}

tcp::socket& session::socket() {
  return socket_;
}

char* session::getData() {
  return data_;
}

void session::start() { // Entryway into session-- session begins first read here.
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred) {
  Logger *log = Logger::get_logger();

  if (!error) {
    log->log_debug("Handling read");
    bool request_found = false;
    string request_string;

    // Add read to working read_string_buffer
    this->read_string_buffer.append(string(data_, 0, bytes_transferred));

    // Parse read_string_buffer for single Request
    int pos = this->read_string_buffer.find("\n\n");
    if (pos != string::npos) {
      request_string = this->read_string_buffer.substr(0, pos + 2);
      request_found = true;
      this->read_string_buffer.erase(0, request_string.length());
    } else {
      pos = this->read_string_buffer.find("\r\n\r\n");
      if (pos != string::npos) { 
        request_string = this->read_string_buffer.substr(0, pos + 4);
        request_found = true;
        this->read_string_buffer.erase(0, request_string.length());
      } else {
        request_found = false;
      }
    }

    if (request_found == true) {
      
      // Feed handlers to handler vector.
      vector<request_handler*> handlers;
      // Parse config file and handle configs dynamically
      parse_config_file("../configuration/handler_config", handlers);

      // (DEBUG) Dump client request to server console.
      log->log_debug("Client requested: " + request_string);

      // Give request_string to handlers to produce response_string.
      string response_string = handle_request(request_string, handlers);

      // (DEBUG) Dump response to server console.
      // cout << "8##" << response_string << "$$$" << endl;

      // Write back to client.
      log->log_debug("Writing back to client with IP: " + socket_.remote_endpoint().address().to_string());
      boost::asio::async_write(socket_,
        boost::asio::buffer(response_string),
        boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));

      // Clean up memory.
      for (int i = 0; i < handlers.size(); i++) {
        delete handlers[i];
      }
    } else { // request_found == false-- just write nothing to client so that we can read again to accumulate read_string_buffer.
      boost::asio::async_write(socket_,
        boost::asio::buffer(string("")),
        boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));
    }
  } else {
    log->log_error("Read handler failed with error code: " + std::to_string(error.value()));
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error) {
  Logger *log = Logger::get_logger();

  if (!error) {
    log->log_debug("Handling write");
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  } else {
    log->log_error("Write handler failed with error code: " + std::to_string(error.value()));
    delete this;
  }
}

// Given request string and vector of request_handler pointers.
// Matches string with appropriate request handler.
// Returns the response string.
string session::handle_request(string request_string, vector<request_handler*> handlers) {
  Logger *log = Logger::get_logger();
  log->log_debug("Handling request");

	string response_string;
	request_handler* rh = new request_handler("", ""); // Default request handler (Always returns 404 NOT FOUND).

	bool found_matching_handler = false;
	for (int i = 0; i < handlers.size(); i++) {
		handlers[i]->set_request_string_(request_string);
		if (handlers[i]->is_matching_handler() == true) {
      log->log_debug("Found matching handler");
			found_matching_handler = true;
			handlers[i]->parse_request(); // Polymorphic call to obtain response_string.
			response_string = handlers[i]->get_response_string_();
		}
	}
	if (found_matching_handler == false) { // Let default handler handle if no matching handler found.
    log->log_debug("No matching handler found, using default");
		rh->set_request_string_(request_string);
		rh->parse_request();
		response_string = rh->get_response_string_();
	}

	// Clean up memory.
	delete rh;

	return response_string;
}

void session::parse_config_file(const string& filename, vector<request_handler*>& handlers) {
  Logger *log = Logger::get_logger();
  ifstream config_file(filename);
  string line;

  while (getline(config_file, line)) {
    // Parse line to extract handler type, URL prefix, and directory path.
    // For example: "static /static1 ../static_files/static_base_directory_1"
    istringstream iss(line);
    string handler_type, url_prefix, directory_path;
    iss >> handler_type >> url_prefix >> directory_path;
    log->log_debug("handler type: " + handler_type);
    log->log_debug("url prefix: " + url_prefix);
    log->log_debug("directory path: " + directory_path);

    // Create handler based on type.
    if (handler_type == "static") {
      static_request_handler* srh = new static_request_handler("", url_prefix, directory_path);
      handlers.push_back(srh);
    } else if (handler_type == "echo") {
      echo_request_handler* erh = new echo_request_handler("", url_prefix);
      handlers.push_back(erh);
    } else {
      // Invalid handler type specified in config file.
      log->log_warn("Invalid handler type specified in config file: " + handler_type);
    }
  }
}
