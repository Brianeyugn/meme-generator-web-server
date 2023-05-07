#ifndef GOOFYGOOGLERSSERVER_LOGGING_H_
#define GOOFYGOOGLERSSERVER_LOGGING_H_

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

// Logger is a singleton that any class 
// can easily access through the static GetLogger()
class Logger {
 public:
  Logger();
  ~Logger();

  static Logger* GetLogger();
  static void SignalHandler(int signal);

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

// Use to query parameters in the code or interpret the algorithm’s steps
inline void Logger::LogTrace(std::string message) {
  BOOST_LOG_TRIVIAL(trace) << message;
}

// Use when giving diagnostic information in a detailed manner
inline void Logger::LogDebug(std::string message) {
  BOOST_LOG_TRIVIAL(debug) << message;
}

// Use during expected situations
inline void Logger::LogInfo(std::string message) {
  BOOST_LOG_TRIVIAL(info) << message;
}

// Use when there is an issue, but the code should continue to work as usual
inline void Logger::LogWarn(std::string message) {
  BOOST_LOG_TRIVIAL(warning) << message;
}

// Use when there is an inability to access a service or a file.
inline void Logger::LogError(std::string message) {
  BOOST_LOG_TRIVIAL(error) << message;
}

// Use when stoping a serious problem or corruption from happening
inline void Logger::LogFatal(std::string message) {
  BOOST_LOG_TRIVIAL(fatal) << message;
}

#endif  // GOOFYGOOGLERSSERVER_LOGGING_H_
