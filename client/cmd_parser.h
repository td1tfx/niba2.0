#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace nibaclient {

class cmd_parser {
public:
    // please design a better api
    // we need to
    // 1. verify correct input
    // 2. return json representation
    static nlohmann::json parse(const std::string& input);

private:
    static std::vector<std::string> splitter(const std::string& s);
};

}