#include "gamedata.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace nibashared;

staticdata &staticdata::get() {
    static staticdata instance;
    return instance;
}

staticdata::staticdata() {
    try {
        std::ifstream char_fin("character.json");
        nlohmann::json serialized_chars = nlohmann::json::parse(char_fin);
        characters_ = serialized_chars.get<decltype(characters_)>();

        std::ifstream magic_fin("magic.json");
        nlohmann::json serialized_magic = nlohmann::json::parse(magic_fin);
        magics_ = serialized_magic.get<decltype(magics_)>();

    } catch (std::exception &e) {
        // I don't care at this point, this should be tested statically
        std::cout << "failed " << e.what() << std::endl;
        exit(-1);
    }
}

// NOTE postgres index start at 1, and we may need to do some manually sequence
// resetting if we get gaps (but that shouldn't be too hard)
const character &staticdata::character(int id) { return characters_.at(id - 1); }

const magic &staticdata::magic(int id) { return magics_.at(id - 1); }