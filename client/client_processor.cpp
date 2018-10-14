#include "client_processor.h"

#include <iostream>

using namespace nibaclient;

client_processor::client_processor()
{
    session.state = nibashared::gamestate::prelogin;
}

void client_processor::process(nibashared::register_request & req)
{
    if (req.success) {
        std::cout << "success" << std::endl;
    }
    else {
        std::cout << "unable to register" << std::endl;
    }
}

void client_processor::process(nibashared::login_request & req)
{
    if (req.success) {
        std::cout << "success" << std::endl;
        session.id = req.id;
        session.state = nibashared::gamestate::selectchar;
        std::cout << "your characters: " << std::endl;
        for (auto& c : req.characters) {
            std::cout << c << std::endl;
        }
    }
    else {
        std::cout << "failed to login" << std::endl;
    }
}

const nibashared::sessionstate & client_processor::get_session()
{
    return session;
}
