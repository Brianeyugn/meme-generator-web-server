#ifndef GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_

#include <string>

#include "request_handler.h"

class StaticRequestHandler : public RequestHandler {
 public:
  StaticRequestHandler(std::string request_string, 
	    std::string handled_directory_name, std::string base_directory_path);
  void ParseRequest() override;
 private:
  std::string base_directory_path_; // Relative path from Server program to base directory.
};

#endif // GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
