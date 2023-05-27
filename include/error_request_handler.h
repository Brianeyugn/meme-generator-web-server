#ifndef ERROR_REQUEST_HANDLER_H
#define ERROR_REQUEST_HANDLER_H

#include "request_handler.h"
#include <boost/beast/http.hpp>

class ErrorRequestHandler : public RequestHandler {
  public:
    ErrorRequestHandler();
    int handle_request(http::request<http::string_body> req, http::response<http::string_body>& res);
};

#endif