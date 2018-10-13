#pragma once
#include "global_defs.h"
#include <nlohmann/json.hpp>
#include <optional>

namespace nibaclient {
    class cmd_processor
    {
    public:
        static std::optional<std::string> handle_cmd(std::string& s);
        static std::optional<std::string> handle_cmd(std::string& s,
            const std::vector<nibashared::cmdtype>& acceptable);            // if empty, allow all
    private:
        static void post_process(nibashared::cmdtype t, nlohmann::json& j);
        static void ask_for_password(nlohmann::json & j);
    };
}


