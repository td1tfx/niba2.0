#include "server_gamedata.h"
#include "db_accessor.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ozo/request.h>
#include <ozo/shortcuts.h>

using namespace nibashared;
using namespace nibaserver;
using namespace ozo::literals;

server_staticdata &server_staticdata::get() {
    static server_staticdata instance;
    return instance;
}

server_staticdata::server_staticdata() {
    // read from postgres here
    const auto connection_info = ozo::make_connection_info("dbname=niba_static user=postgres");
    ozo::io_context ioc;
    const auto connector = ozo::make_connector(connection_info, ioc);
    ozo::rows_of<std::string> character, magic, item;

    boost::asio::spawn(ioc, [&] (boost::asio::yield_context yield) {
        ozo::request(connector, "SELECT to_json::TEXT FROM character_dump"_SQL, 
            ozo::into(character), yield);

        ozo::request(connector, "SELECT to_json::TEXT FROM magic_dump"_SQL, 
            ozo::into(magic), yield);

        ozo::request(connector, "SELECT to_json::TEXT FROM item_dump"_SQL, 
            ozo::into(item), yield);
    });
    ioc.run();

    try{
        nlohmann::json serialized_chars = nlohmann::json::parse(std::get<0>(character.at(0)));
        for (auto &element : serialized_chars) {
            characters_[element["character_id"]] = element;
        }

        nlohmann::json serialized_magic = nlohmann::json::parse(std::get<0>(magic.at(0)));
        for (auto &element : serialized_magic) {
            magics_[element["magic_id"]] = element;
        }

        nlohmann::json serialized_equipment = nlohmann::json::parse(std::get<0>(item.at(0)));
        for (auto &element : serialized_equipment) {
            equipments_[element["equipment_id"]] = element;
        }
    } catch (std::exception &e) {
        // I don't care at this point, this should be tested statically
        std::cout << "failed " << e.what() << std::endl;
        exit(-1);
    }
}

// NOTE postgres index start at 1, and we may need to do some manually sequence
// resetting if we get gaps (but that shouldn't be too hard)
const nibashared::character &server_staticdata::character(int id) { return characters_.at(id); }

const nibashared::magic &server_staticdata::magic(int id) { return magics_.at(id); }

const nibashared::equipment &server_staticdata::equipment(int id) { return equipments_.at(id); }
