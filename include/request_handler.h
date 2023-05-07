#ifndef GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_
#define GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_

#include <string>

/* RequestHandler Ussage Notes */
// After constructing handler object.
// Must SetRequestString_() first.
// Then can check IsMatchingHandler().
// Or Then can also ParseRequest().

class RequestHandler {
 public:
  RequestHandler(std::string request_string, std::string handled_directory_name);

  bool IsMatchingHandler();
  virtual void ParseRequest();

  static std::string GetNextToken(std::string str, std::string delimiter);
  static std::string GetRequestURL(std::string request_string);

  // Getters.
  std::string GetResponseString_();
  std::string GetRequestString_();

  // Setters.
  void SetRequestString(std::string request_string);
  void SetResponseString(std::string response_string);

 protected:
  std::string request_string_;
  std::string handled_directory_name_;
  std::string response_string_;
};

// Must ParseRequest() first, then GetResponseString_().
inline std::string RequestHandler::GetResponseString_() {
  return this->response_string_;
}

inline std::string RequestHandler::GetRequestString_() {
  return this->request_string_;
}

inline void RequestHandler::SetRequestString(std::string request_string) {
  this->request_string_ = request_string;
}

inline void RequestHandler::SetResponseString(std::string response_string) {
  this->response_string_ = response_string;
}

#endif  // GOOFYGOOGLERSSERVER_REQUEST_HANDLER_H_