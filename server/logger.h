#pragma once

#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace nibaserver {

void init_log();

class logger : public boost::log::sources::severity_logger<boost::log::trivial::severity_level> {
public:
    logger(){};
    ~logger(){};
};

} // namespace nibaserver
