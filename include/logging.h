#ifndef GOOFYGOOGLERSSERVER_LOGGING_H_
#define GOOFYGOOGLERSSERVER_LOGGING_H_

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

namespace logging = boost::log;

class Logger {
 public:
  Logger();

};

#endif  // GOOFYGOOGLERSSERVER_LOGGING_H_
