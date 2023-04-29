#ifndef GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_

#include <string>

#include "request_handler.h"

using namespace std;

class static_request_handler : public request_handler
{
public:
	static_request_handler(string request_string, string handled_directory_name, string base_directory_path);
	void parse_request() override;
	static string get_filename(string request_url);
	static string get_content_type(string extension);
private:
	string base_directory_path_; // Relative path from server program to base directory.
};

#endif // GOOFYGOOGLERSSERVER_STATIC_REQUEST_HANDLER_H_
