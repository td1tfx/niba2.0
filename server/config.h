#pragma once

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

namespace nibaserver {

struct config {
    std::string host;
    unsigned short port;
    int threads;
    std::string static_conn_str;
    std::string player_conn_str;
};

inline config read_config(int argc, char *argv[]) {
    config my_config = {.host = "0.0.0.0",
                        .port = 19999,
                        .threads = 2,
                        .static_conn_str = "dbname=niba_static user=postgres",
                        .player_conn_str = "dbname=niba user=postgres"};
    if (argc < 2) {
        return my_config;
    }
    try {
        std::ifstream fs(argv[1]);
        nlohmann::json json = nlohmann::json::parse(fs);
        if (auto iter = json.find("host"); iter != json.end()) {
            iter->get_to(my_config.host);
        }
        if (auto iter = json.find("port"); iter != json.end()) {
            iter->get_to(my_config.port);
        }
        if (auto iter = json.find("threads"); iter != json.end()) {
            iter->get_to(my_config.threads);
        }
        if (auto iter = json.find("static_conn_str"); iter != json.end()) {
            iter->get_to(my_config.static_conn_str);
        }
        if (auto iter = json.find("player_conn_str"); iter != json.end()) {
            iter->get_to(my_config.player_conn_str);
        }
    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }
    return my_config;
}

} // namespace nibaserver