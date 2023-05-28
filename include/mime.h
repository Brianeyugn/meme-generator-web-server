#ifndef GOOFYGOOGLERSSERVER_MIME_H_
#define GOOFYGOOGLERSSERVER_MIME_H_

#include <string>
#include <map>

class Mime {
 public:
  Mime();
  std::string getContentType(std::string fileExt);
  
 private:
  std::map<const std::string, std::string> content_type_ = {
    {"txt", "text/plain"},
    {"html", "text/html"},
    {"htm", "text/html"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"zip", "application/zip"}
  };
};

#endif // GOOFYGOOGLERSSERVER_MIME_H_