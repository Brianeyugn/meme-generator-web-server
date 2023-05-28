#include "mime.h"

Mime::Mime() {}

std::string Mime::getContentType(std::string fileExt) {
  std::map<const std::string, std::string>::iterator it;
  it = content_type_.find(fileExt);
  if (it == content_type_.end()) {
    return "application/octet-stream";
  } else {
    return it->second;
  }
}
