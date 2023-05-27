#include "echo_request_handler.h"
#include "request_handler.h"

#include <boost/lexical_cast.hpp>

EchoRequestHandler::EchoRequestHandler()
	: RequestHandler() {
}

int EchoRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  if(req.method_string() == "") {
    res.reason("Bad Request");
    res.result(400);
    return 400;
  }

  res.version(req.version());
  res.reason("OK");
  res.result(200);
  res.set(http::field::content_type, "text/plain");

  std::string const string_headers = boost::lexical_cast<std::string>(req.base());
  std::string body;
  body += string_headers;
  body += req.body();
  res.body() = body;

  res.set(http::field::content_length, std::to_string(res.body().size()));

  return 200;
}