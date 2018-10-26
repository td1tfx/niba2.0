#include "logger.h"

using namespace nibaserver;
namespace logging = boost::log;

void nibaserver::init_log() {
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");
    logging::add_common_attributes();
    logging::add_file_log(
        keywords::file_name = "logs/server_%N.log",
        keywords::rotation_size = 10 * 1024 * 1024,
        keywords::time_based_rotation = logging::sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::auto_flush = true, keywords::format = "[%TimeStamp%]: [%Severity%] %Message%");
}
