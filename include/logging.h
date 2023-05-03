#ifndef GOOFYGOOGLERSSERVER_LOGGING_H_
#define GOOFYGOOGLERSSERVER_LOGGING_H_

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

namespace logging = boost::log;

class Logger {
 public:
  Logger();
  ~Logger();

  static Logger* get_logger() {
    if (Logger::logger_ == nullptr) {
      Logger::logger_ = new Logger();
    }

    return Logger::logger_;
  }

  static void signal_handler(int signal) {
    BOOST_LOG_TRIVIAL(fatal) << "Server interrupted with signal: " << signal;
    exit(signal);
  }

  void log_trace(std::string message);
  void log_debug(std::string message);
  void log_info (std::string message);
  void log_warn (std::string message);
  void log_error(std::string message);
  void log_fatal(std::string message);

 private:
  // static allows us to use the same Logger object across multiple files
  static Logger *logger_;
};

#endif  // GOOFYGOOGLERSSERVER_LOGGING_H_
