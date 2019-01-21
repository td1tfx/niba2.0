#include "client_gamedata.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace nibashared;

client_staticdata &client_staticdata::get() {
    static client_staticdata instance;
    return instance;
}

client_staticdata::client_staticdata() {
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
const character &client_staticdata::character(int id) { return characters_.at(id); }

const magic &client_staticdata::magic(int id) { return magics_.at(id); }

const equipment &client_staticdata::equipment(int id) { return equipments_.at(id); }
