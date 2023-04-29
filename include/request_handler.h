#ifndef GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_

#include <string>

using namespace std;

/* request_handler Ussage Notes */
// After constructing handler object.
// Must set_request_string_() first.
// Then can check is_matching_handler().
// Or Then can also parse_request().

class request_handler {
public:
  request_handler(string request_string, string handled_directory_name);
  static string get_next_token(string str, string delimiter);
  static string get_request_url(string request_string);
  bool is_matching_handler();
  virtual void parse_request();

  // Getters.
  string get_response_string_(); // Note: must parse_request() first, then get_response_string_().
  string get_request_string_();

  // Setters.
  void set_request_string_(string request_string);
  void set_response_string_(string response_string);
protected:
  string request_string_;
  string handled_directory_name_;
  string response_string_;

};

#endif // GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_