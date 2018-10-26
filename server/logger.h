#pragma once

#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;
namespace sev = boost::log::trivial;

namespace nibaserver {

void init_log();

class logger : public src::severity_logger<sev::severity_level> {
public:
    logger(){};
    ~logger(){};
};

} // namespace nibaserver
