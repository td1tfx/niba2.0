#pragma once
#include "global_defs.h"
#include "client_request.h"

namespace nibaclient {
    class client_processor
    {
    public:
        client_processor();
        ~client_processor() = default;
        void process(nibashared::register_request& req);
        void process(nibashared::login_request& req);
        const nibashared::sessionstate& get_session();

    private:
        nibashared::sessionstate session;
    };
}
