#include "cmd_processor.h"
#include <iostream>
#include <boost/algorithm/string.hpp>


using namespace nibaclient;


std::vector<std::string> nibaclient::cmd_processor::splitter(const std::string & s)
{
    std::vector<std::string> result;
    boost::split(result, s, boost::is_any_of("\t "));
    return result;
}