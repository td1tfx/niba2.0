// this file is generated, see structs.yaml and gen_structs.py

#include "structs.h"

namespace nibashared {

void to_json(nlohmann::json &j, const battlestats &battlestats) {
    j = nlohmann::json{
        {"hp", battlestats.hp},
        {"mp", battlestats.mp},
        {"attack_min", battlestats.attack_min},
        {"attack_max", battlestats.attack_max},
        {"inner_power", battlestats.inner_power},
        {"accuracy", battlestats.accuracy},
        {"evasion", battlestats.evasion},
        {"speed", battlestats.speed},
        {"defence", battlestats.defence},
        {"crit_chance", battlestats.crit_chance},
        {"crit_damage", battlestats.crit_damage},
        {"reduce_def", battlestats.reduce_def},
        {"reduce_def_perc", battlestats.reduce_def_perc},
        {"hp_regen", battlestats.hp_regen},
        {"mp_regen", battlestats.mp_regen},
        {"gold_res", battlestats.gold_res},
        {"wood_res", battlestats.wood_res},
        {"water_res", battlestats.water_res},
        {"fire_res", battlestats.fire_res},
        {"earth_res", battlestats.earth_res},
        {"hp_on_hit", battlestats.hp_on_hit},
        {"hp_steal", battlestats.hp_steal},
        {"mp_on_hit", battlestats.mp_on_hit},
        {"mp_steal", battlestats.mp_steal},
    };
}

void from_json(const nlohmann::json &j, battlestats &battlestats) {
    j.at("hp").get_to(battlestats.hp);
    j.at("mp").get_to(battlestats.mp);
    j.at("attack_min").get_to(battlestats.attack_min);
    j.at("attack_max").get_to(battlestats.attack_max);
    j.at("inner_power").get_to(battlestats.inner_power);
    j.at("accuracy").get_to(battlestats.accuracy);
    j.at("evasion").get_to(battlestats.evasion);
    j.at("speed").get_to(battlestats.speed);
    j.at("defence").get_to(battlestats.defence);
    j.at("crit_chance").get_to(battlestats.crit_chance);
    j.at("crit_damage").get_to(battlestats.crit_damage);
    j.at("reduce_def").get_to(battlestats.reduce_def);
    j.at("reduce_def_perc").get_to(battlestats.reduce_def_perc);
    j.at("hp_regen").get_to(battlestats.hp_regen);
    j.at("mp_regen").get_to(battlestats.mp_regen);
    j.at("gold_res").get_to(battlestats.gold_res);
    j.at("wood_res").get_to(battlestats.wood_res);
    j.at("water_res").get_to(battlestats.water_res);
    j.at("fire_res").get_to(battlestats.fire_res);
    j.at("earth_res").get_to(battlestats.earth_res);
    j.at("hp_on_hit").get_to(battlestats.hp_on_hit);
    j.at("hp_steal").get_to(battlestats.hp_steal);
    j.at("mp_on_hit").get_to(battlestats.mp_on_hit);
    j.at("mp_steal").get_to(battlestats.mp_steal);
}

void to_json(nlohmann::json &j, const attributes &attributes) {
    j = nlohmann::json{
        {"strength", attributes.strength},
        {"dexterity", attributes.dexterity},
        {"physique", attributes.physique},
        {"spirit", attributes.spirit},
    };
}

void from_json(const nlohmann::json &j, attributes &attributes) {
    j.at("strength").get_to(attributes.strength);
    j.at("dexterity").get_to(attributes.dexterity);
    j.at("physique").get_to(attributes.physique);
    j.at("spirit").get_to(attributes.spirit);
}

void to_json(nlohmann::json &j, const character &character) {
    j = nlohmann::json{
        {"name", character.name},
        {"character_id", character.character_id},
        {"attrs", character.attrs},
        {"stats", character.stats},
        {"equipments", character.equipments},
        {"active_magic", character.active_magic},
    };
}

void from_json(const nlohmann::json &j, character &character) {
    j.at("name").get_to(character.name);
    j.at("character_id").get_to(character.character_id);
    j.at("attrs").get_to(character.attrs);
    j.at("stats").get_to(character.stats);
    j.at("equipments").get_to(character.equipments);
    j.at("active_magic").get_to(character.active_magic);
}

void to_json(nlohmann::json &j, const magic &magic) {
    j = nlohmann::json{
        {"name", magic.name},
        {"magic_id", magic.magic_id},
        {"active", magic.active},
        {"cd", magic.cd},
        {"multiplier", magic.multiplier},
        {"inner_damage", magic.inner_damage},
        {"mp_cost", magic.mp_cost},
        {"inner_property", magic.inner_property},
        {"stats", magic.stats},
    };
}

void from_json(const nlohmann::json &j, magic &magic) {
    j.at("name").get_to(magic.name);
    j.at("magic_id").get_to(magic.magic_id);
    j.at("active").get_to(magic.active);
    j.at("cd").get_to(magic.cd);
    j.at("multiplier").get_to(magic.multiplier);
    j.at("inner_damage").get_to(magic.inner_damage);
    j.at("mp_cost").get_to(magic.mp_cost);
    j.at("inner_property").get_to(magic.inner_property);
    j.at("stats").get_to(magic.stats);
}

void to_json(nlohmann::json &j, const equipment &equipment) {
    j = nlohmann::json{
        {"equipment_id", equipment.equipment_id},
        {"name", equipment.name},
        {"description", equipment.description},
        {"type", equipment.type},
        {"stats", equipment.stats},
        {"item_level", equipment.item_level},
        {"required_level", equipment.required_level},
    };
}

void from_json(const nlohmann::json &j, equipment &equipment) {
    j.at("equipment_id").get_to(equipment.equipment_id);
    j.at("name").get_to(equipment.name);
    j.at("description").get_to(equipment.description);
    j.at("type").get_to(equipment.type);
    j.at("stats").get_to(equipment.stats);
    j.at("item_level").get_to(equipment.item_level);
    j.at("required_level").get_to(equipment.required_level);
}

} // namespace nibashared