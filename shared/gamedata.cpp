#include "gamedata.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace nibashared;

staticdata &staticdata::get(bool is_server, nibaserver::db_accessor *db_ptr) {
    static staticdata instance(is_server, db_ptr);
    return instance;
}

staticdata::staticdata(bool is_server, nibaserver::db_accessor *db_ptr) {
    if (is_server) {
        staticdata::init_static_data_from_postgres(*db_ptr); 
    } else {
        staticdata::init_static_data_from_json();
    }
}

void staticdata::init_static_data_from_postgres(nibaserver::db_accessor db) {
    std::cout << "init_static_data_from_postgres\n";
    // to keep the function complete at the moment
    staticdata::init_static_data_from_json();
}; 

void staticdata::init_static_data_from_json() {
    try {
        std::ifstream char_fin("character.json");
        nlohmann::json serialized_chars = nlohmann::json::parse(char_fin);
        for (auto &element : serialized_chars) {
            characters_[element["character_id"]] = element;
        }

        std::ifstream magic_fin("magic.json");
        nlohmann::json serialized_magic = nlohmann::json::parse(magic_fin);
        for (auto &element : serialized_magic) {
            magics_[element["magic_id"]] = element;
        }

        std::ifstream equipment_fin("equipment.json");
        nlohmann::json serialized_equipment = nlohmann::json::parse(equipment_fin);
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
const character &staticdata::character(int id) { return characters_.at(id); }

const magic &staticdata::magic(int id) { return magics_.at(id); }

const equipment &staticdata::equipment(int id) { return equipments_.at(id); }
