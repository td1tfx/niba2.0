#include "cmd_processor.h"
#include "cmd_parser.h"
#include <iostream>

using namespace nibaclient;

std::optional<std::string> nibaclient::cmd_processor::handle_cmd(std::string & s)
{
    return handle_cmd(s, {});
}

std::optional<std::string> nibaclient::cmd_processor::handle_cmd(std::string & s, 
    const std::vector<nibashared::cmdtype>& acceptable)
{
    try {
        auto j_str = cmd_parser::parse(s);
        nibashared::cmdtype t = j_str["type"];
        if (!acceptable.empty()) {
            auto iter = std::find(acceptable.begin(), acceptable.end(), t);
            if (iter == acceptable.end()) {
                throw std::exception("command not allowed");
            }
        }
        post_process(t, j_str);
        return j_str.dump();
    }
    // change to whatever better exception
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return {};
    }
}

void nibaclient::cmd_processor::post_process(nibashared::cmdtype t, nlohmann::json & j)
{
    switch (t) {
    case nibashared::cmdtype::login:
    case nibashared::cmdtype::registeration:
        ask_for_password(j);
    }
}

void nibaclient::cmd_processor::ask_for_password(nlohmann::json & j)
{
    // hide password magic
    std::cout << "what's your password?" << std::endl;
    std::string pswd;
    std::getline(std::cin, pswd);
    j["password"] = pswd;
    // ssl, transport by plaintext is ok
}
