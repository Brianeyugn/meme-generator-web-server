#include "logging.h"

#include <cstdio>

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

#define LOG_FILE_NAME   "../logs/server_%Y-%m-%d_%H-%M-%S.%N.log"
#define LOG_FILE_FORMAT "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%"

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

Logger *Logger::logger_ = nullptr;

Logger::Logger() {
  // Some Logger object already exists
  if (logger_ != nullptr) {
    fprintf(stderr, "Cannot instantiate new Logger object as one already exists");
    exit(-1);
  }

  Logger *logger_ = this;

  logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

  // Output logs to file (debug level is not written to file)
  logging::add_file_log(
    keywords::file_name = LOG_FILE_NAME,
    keywords::format = LOG_FILE_FORMAT,
    keywords::rotation_size = 10 * 1024 * 1024, // Rotate after 10Mb
    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), // New log after midnight
    keywords::auto_flush = true
  )->set_filter(
    logging::trivial::severity >= logging::trivial::info
  );

  // Output logs to console
  logging::add_console_log(
    std::cout,
    keywords::format = LOG_FILE_FORMAT,
    keywords::auto_flush = true);

  logging::add_common_attributes();
}

Logger::~Logger() {
  logger_ = nullptr;
}

Logger* Logger::GetLogger() {
  // If a singleton logger doesn't exist yet, create one. 
  // Otherwise return the original
  if (Logger::logger_ == nullptr) {
    Logger::logger_ = new Logger();
  }

  return Logger::logger_;
}

void Logger::SignalHandler(int signal) {
  BOOST_LOG_TRIVIAL(fatal) << "Server interrupted with signal: " << signal;
  exit(signal);
}
