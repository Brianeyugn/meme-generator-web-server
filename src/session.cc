#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream> // For Debugging
#include <cstring> // For strcpy
#include <sstream> // For iostringstream
#include <iostream>

#include "session.h"
#include "request_handler.h"
#include "echo_request_handler.h"
#include "static_request_handler.h"

using boost::asio::ip::tcp;
using namespace std;

session::session(boost::asio::io_service& io_service)
  : socket_(io_service) {
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
  if (!error) {
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
      // Setup handlers
      echo_request_handler* erh1 = new echo_request_handler("", "echo");
      string base_1 = "../static_files/static_base_directory_1";
      string base_2 = "../static_files/static_base_directory_2";
      static_request_handler* srh1 = new static_request_handler("", "static1", base_1);
      static_request_handler* srh2 = new static_request_handler("", "static2", base_2);

      // Feed handlers to handler vector.
      vector<request_handler*> handlers;
      handlers.push_back(erh1);
      handlers.push_back(srh1);
      handlers.push_back(srh2);

      // (DEBUG) Dump client request to server console.
      cout << request_string << endl;

      // Give request_string to handlers to produce response_string.
      string response_string = handle_request(request_string, handlers);

      // (DEBUG) Dump response to server console.
      // cout << "8##" << response_string << "$$$" << endl;

      // Write back to client.
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
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error) {
  if (!error) {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  } else {
    delete this;
  }
}

// Given request string and vector of request_handler pointers.
// Matches string with appropriate request handler.
// Returns the response string.
string session::handle_request(string request_string, vector<request_handler*> handlers) {
	string response_string;

	request_handler* rh = new request_handler("", ""); // Default request handler (Always returns 404 NOT FOUND).

	bool found_matching_handler = false;
	for (int i = 0; i < handlers.size(); i++) {
		handlers[i]->set_request_string_(request_string);
		if (handlers[i]->is_matching_handler() == true)
		{
			found_matching_handler = true;
			handlers[i]->parse_request(); // Polymorphic call to obtain response_string.
			response_string = handlers[i]->get_response_string_();
		}
	}
	if (found_matching_handler == false) // Let default handler handle if no matching handler found.
	{
		rh->set_request_string_(request_string);
		rh->parse_request();
		response_string = rh->get_response_string_();
	}

	// Clean up memory.
	delete rh;

	return response_string;
}
