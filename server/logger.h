#pragma once

#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/format.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

namespace nibaserver {

void init_log();

class logger {
public:
    logger(){};
    ~logger(){};
    void log(std::string log_msg, logging::trivial::severity_level sev = logging::trivial::info);

private:
    src::severity_logger<logging::trivial::severity_level> log_handle;
};
} // namespace nibaserver
