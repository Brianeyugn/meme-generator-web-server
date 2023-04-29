#ifndef GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_

#include "request_handler.h"

class echo_request_handler : public request_handler {
public:
	echo_request_handler(string request_string, string handled_directory_name);
	void parse_request() override;
private:
};

#endif //GOOFYGOOGLERSSERVER_ECHO_REQUEST_HANDLER_H_
