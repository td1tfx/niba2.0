#include "server_processor.h"
#include "db_accessor.h"

#include <iostream>

using namespace nibaserver;

server_processor::server_processor() { session.state = nibashared::gamestate::prelogin; }

void nibaserver::server_processor::process(nibashared::register_request &req) {
    if (db_accessor::create_user(req.id, req.password)) {
        req.success = true;
    } else {
        req.success = false;
    }
    std::cout << "register " << req.success << std::endl;
}

void nibaserver::server_processor::process(nibashared::login_request &req) {
    if (db_accessor::login(req.id, req.password)) {
        req.success = true;
        session.state = nibashared::gamestate::selectchar;
        session.id = req.id;
        req.characters = {"niba1", "niba2"};
    } else {
        req.success = false;
    }
    std::cout << "login " << req.success << std::endl;
}

const nibashared::sessionstate &nibaserver::server_processor::get_session() { return session; }
