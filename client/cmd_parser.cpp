#include "cmd_parser.h"
#include "global_defs.h"

#include <boost/algorithm/string.hpp>

using namespace nibaclient;
using json = nlohmann::json;


nlohmann::json nibaclient::cmd_parser::parse(const std::string & input)
{
    auto results = splitter(input);
    // password input handled by cmd_processor
    if (results.size() == 2) {
        if (results[0] == "register") {
            return { {"type", nibashared::cmdtype::registeration}, {"id", results[1]}, {"password", ""} };
        }
        else if (results[0] == "login") {
            return { {"type", nibashared::cmdtype::login}, {"id", results[1]}, {"password", ""} };
        }
    }
    throw std::exception("incorrect command");
}

std::vector<std::string> nibaclient::cmd_parser::splitter(const std::string & s)
{
    std::vector<std::string> result;
    boost::split(result, s, boost::is_any_of("\t "));
    return result;
}
