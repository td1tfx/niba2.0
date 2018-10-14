#include "utils.h"

bool nibashared::is_cmd_valid(nibashared::gamestate state, nibashared::cmdtype cmd) {
    // given a state, what are the acceptable cmds
    static const std::array<std::vector<nibashared::cmdtype>, to_underlying(nibashared::gamestate::LAST)> state_cmd{
        // prelogin
        std::vector<nibashared::cmdtype>{ nibashared::cmdtype::login, nibashared::cmdtype::registeration },
        // selectchar
        std::vector<nibashared::cmdtype>{ nibashared::cmdtype::create, nibashared::cmdtype::start }
    };
    const auto& cmds = state_cmd.at(to_underlying(state));
    return std::find(cmds.begin(), cmds.end(), cmd) != cmds.end();
}