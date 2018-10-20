#pragma once
#include "global_defs.h"
#include "message.h"
#include <nlohmann/json.hpp>

namespace nibaclient {
class client_processor {
public:
    client_processor();
    ~client_processor() = default;
    void process(nibashared::register_request &req);
    void process(nibashared::login_request &req);
    const nibashared::sessionstate &get_session();

    template<typename message>
    void dispatch(message &m, const std::string &merger) {
        try {
            auto merge_j = nlohmann::json::parse(merger);
            if (merge_j.find("error") != merge_j.end()) {
                std::string err = merge_j["error"].get<std::string>();
                throw std::exception(err.c_str());
            }
            m.merge_response(merge_j);
            process(m);
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }

private:
    nibashared::sessionstate session;
};
} // namespace nibaclient
