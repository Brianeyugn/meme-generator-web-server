#include "logging.h"

namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

Logger::Logger() {
  logging::add_file_log(
    keywords::file_name = "../logs/server_%Y-%m-%d_%H-%M-%S.%N.log",
    keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
    keywords::rotation_size = 10 * 1024 * 1024, // rotate after 10Mb
    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), // new log after midnight
    keywords::auto_flush = true
  );
}
