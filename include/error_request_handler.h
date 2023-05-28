#ifndef GOOFYGOOGLERSSERVER_ERROR_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_ERROR_REQUEST_HANDLER_H_

#include "request_handler.h"
#include <boost/beast/http.hpp>

class ErrorRequestHandler : public RequestHandler {
 public:
  ErrorRequestHandler();
  int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);
};

#endif // GOOFYGOOGLERSSERVER_ERROR_REQUEST_HANDLER_H_
