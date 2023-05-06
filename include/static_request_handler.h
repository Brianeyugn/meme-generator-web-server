#ifndef GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_

#include <string>

#include "request_handler.h"

using namespace std;

class StaticRequestHandler : public RequestHandler
{
 public:
  StaticRequestHandler(string request_string, 
	    string handled_directory_name, string base_directory_path);
  void ParseRequest() override;
  static string GetFilename(string request_url);
  static string GetContentType(string extension);
 private:
  string base_directory_path_; // Relative path from Server program to base directory.
};

#endif // GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
