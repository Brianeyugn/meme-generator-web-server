#ifndef GOOFYGOOGLERSSERVER_LOGGING_H_
#define GOOFYGOOGLERSSERVER_LOGGING_H_

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

namespace logging = boost::log;

// Logger is a singleton that any class 
// can easily access through the static GetLogger()
class Logger {
 public:
  Logger();
  ~Logger();

  static Logger* GetLogger() {
    // If a singleton logger doesn't exist yet, create one. 
    // Otherwise return the original
    if (Logger::logger_ == nullptr) {
      Logger::logger_ = new Logger();
    }

    return Logger::logger_;
  }

  static void SignalHandler(int signal) {
    BOOST_LOG_TRIVIAL(fatal) << "Server interrupted with signal: " << signal;
    exit(signal);
  }

  void LogTrace(std::string message);
  void LogDebug(std::string message);
  void LogInfo (std::string message);
  void LogWarn (std::string message);
  void LogError(std::string message);
  void LogFatal(std::string message);

 private:
  // static allows us to use the same Logger object across multiple files
  static Logger *logger_;
};

#endif  // GOOFYGOOGLERSSERVER_LOGGING_H_
