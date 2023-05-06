#ifndef GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_

#include <string>

using namespace std;

/* RequestHandler Ussage Notes */
// After constructing handler object.
// Must SetRequestString_() first.
// Then can check IsMatchingHandler().
// Or Then can also ParseRequest().

class RequestHandler {
 public:
  RequestHandler(string request_string, string handled_directory_name);
  static string GetNextToken(string str, string delimiter);
  static string GetRequestURL(string request_string);
  bool IsMatchingHandler();
  virtual void ParseRequest();

  // Getters.
  string GetResponseString_();
  string GetRequestString_();

  // Setters.
  void SetRequestString(string request_string);
  void SetResponseString(string response_string);
 protected:
  string request_string_;
  string handled_directory_name_;
  string response_string_;

};

#endif  // GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_