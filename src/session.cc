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
#include "error_request_handler.h"
#include "logging.h"
#include "request_handler.h"
#include "static_request_handler.h"
#include "api_request_handler.h"
#include "request_factory.h"

namespace http = boost::beast::http;
using boost::asio::ip::tcp;

Session ::Session(boost::asio::io_service& io_service, 
                std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map,
                std::map<std::string, RequestHandlerFactory*> routes)
        : socket_(io_service) {
  handler_map_ = handler_map;
  routes_ = routes;
}

RealSession ::RealSession(boost::asio::io_service& io_service,
                        std::map<std::string, std::pair<std::string, NginxConfig*>> handler_map,
                        std::map<std::string, RequestHandlerFactory*> routes)
            : Session(io_service, handler_map, routes) {
}

tcp::socket& RealSession::Socket() {
  return socket_;
}

// Entryway into Session-- Session begins first read here.
void RealSession::Start() {
  client_ip_ = socket_.remote_endpoint().address().to_string();
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&RealSession::HandleRead, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

void RealSession::HandleRead(const boost::system::error_code& error,
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

  std::string msg = std::string(data_);

  http::request_parser<http::string_body> parser;

  boost::beast::error_code ec;
  boost::asio::const_buffer request_buffer(msg.data(), msg.size());
  parser.put(request_buffer, ec);

  http::request<http::string_body> req;
  http_.getFields(msg, req);

  std::string target(req.target().begin(), req.target().end());
  std::string location = match(target);

  http::response<http::string_body> res;

  if ((ec && ec != http::error::bad_version)) {
    res.result(400);
    res.reason("Bad Request");
  } else {
    int status;
    if (location != "") {
      log->LogInfo("RealSession :: handle_read found longest match = " + location + "\n");
      RequestHandlerFactory* factory = routes_[location];
      NginxConfig* conf = handler_map_[location].second;
      RequestHandler* handler = factory->create(location, conf);
      status = handler->handle_request(req, res);
    } else {
      location = "/";
      RequestHandlerFactory* factory = routes_[location];
      NginxConfig* conf = handler_map_[location].second;
      RequestHandler* handler = factory->create(location, conf);
      status = handler->handle_request(req, res);
    }
  }

  memset(data_, 0, sizeof data_);

  std::string response_string = http_.getResponse(res);

  std::vector<char>response_(response_string.begin(), response_string.end());
  int res_bytes = response_string.length();

  boost::asio::async_write(socket_,
    boost::asio::buffer(response_, res_bytes),
    boost::bind(&RealSession::HandleWrite, this,
    boost::asio::placeholders::error));
}

void RealSession::HandleWrite(const boost::system::error_code& error) {
  Logger *log = Logger::GetLogger();

  if (error) {
    log->LogError("Write handler failed with error code: " + std::to_string(error.value()));
    delete this;
    return;
  } 

  log->LogDebug("Handling write");
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
    boost::bind(&RealSession::HandleRead, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
}

// Given request string 
// Matches string with appropriate request handler.
// Returns the response string.
std::string RealSession::match(std::string request_uri) {
  Logger* log = Logger::GetLogger();
  log->LogInfo("match :: request_uri = " + request_uri + "\n");
  std::string res = "";
  int res_len = 0;
  std::map<std::string, RequestHandlerFactory*>::iterator it;
  for (it = routes_.begin(); it != routes_.end(); it++) {
    std::string location = it->first;
    log->LogInfo("match :: location = " + location + "\n");
    if (request_uri.substr(0, location.length()) == location &&
       (request_uri.length() == location.length() ||
        request_uri.at(location.length()) == '/')) {
        if (location.length() > res_len) {
          res = location;
          res_len = location.length();
        }
      }
  }
  log->LogInfo("match :: result = " + res + "\n");
  return res;
}
