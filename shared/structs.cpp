#include "structs.h"

// pretty much all serialization for structs to json
// ideally this should be generated

void nibashared::to_json(nlohmann::json &j, const battlestats &stats) {
    j = nlohmann::json{
        {"attack_min", stats.attack_min},
        {"attack_max", stats.attack_max},
        {"accuracy", stats.accuracy},
        {"evasion", stats.evasion},
        {"speed", stats.speed},
        {"hp", stats.hp},
        {"mp", stats.mp},
        {"inner_power", stats.inner_power},
        {"defence", stats.defence},
        {"crit_chance", stats.crit_chance},
        {"crit_damage", stats.crit_damage},
        {"reduce_def", stats.reduce_def},
        {"reduce_def_perc", stats.reduce_def_perc},
        {"hp_regen", stats.hp_regen},
        {"mp_regen", stats.mp_regen},
        {"gold_res", stats.gold_res},
        {"wood_res", stats.wood_res},
        {"water_res", stats.water_res},
        {"fire_res", stats.fire_res},
        {"earth_res", stats.earth_res},
        {"hp_on_hit", stats.hp_on_hit},
        {"hp_steal", stats.hp_steal},
        {"mp_on_hit", stats.mp_on_hit},
        {"mp_steal", stats.mp_steal},
    };
}

void nibashared::from_json(const nlohmann::json &j, battlestats &stats) {
    j.at("attack_min").get_to(stats.attack_min);
    j.at("attack_max").get_to(stats.attack_max);
    j.at("accuracy").get_to(stats.accuracy);
    j.at("evasion").get_to(stats.evasion);
    j.at("speed").get_to(stats.speed);
    j.at("hp").get_to(stats.hp);
    j.at("mp").get_to(stats.mp);
    j.at("inner_power").get_to(stats.inner_power);
    j.at("crit_chance").get_to(stats.crit_chance);
    j.at("crit_damage").get_to(stats.crit_damage);
    j.at("reduce_def").get_to(stats.reduce_def);
    j.at("reduce_def_perc").get_to(stats.reduce_def_perc);
    j.at("hp_regen").get_to(stats.hp_regen);
    j.at("mp_regen").get_to(stats.mp_regen);
    j.at("gold_res").get_to(stats.gold_res);
    j.at("wood_res").get_to(stats.wood_res);
    j.at("water_res").get_to(stats.water_res);
    j.at("fire_res").get_to(stats.fire_res);
    j.at("earth_res").get_to(stats.earth_res);
    j.at("hp_on_hit").get_to(stats.hp_on_hit);
    j.at("hp_steal").get_to(stats.hp_steal);
    j.at("mp_on_hit").get_to(stats.mp_on_hit);
    j.at("mp_steal").get_to(stats.mp_steal);
}

void nibashared::to_json(nlohmann::json &j, const attributes &attr) {
    j = nlohmann::json{{"strength", attr.strength},
                       {"dexterity", attr.dexterity},
                       {"physique", attr.physique},
                       {"spirit", attr.spirit}};
}

void nibashared::from_json(const nlohmann::json &j, attributes &attr) {
    j.at("strength").get_to(attr.strength);
    j.at("dexterity").get_to(attr.dexterity);
    j.at("physique").get_to(attr.physique);
    j.at("spirit").get_to(attr.spirit);
}

void nibashared::to_json(nlohmann::json &j, const character &character) {
    j = nlohmann::json{{"name", character.name},
                       {"character_id", character.character_id},
                       {"active_magic", character.active_magic},
                       {"attrs", character.attrs}};
}

void nibashared::from_json(const nlohmann::json &j, character &character) {
    j.at("name").get_to(character.name);
    j.at("character_id").get_to(character.character_id);
    j.at("active_magic").get_to(character.active_magic);
    j.at("attrs").get_to(character.attrs);
}

void nibashared::to_json(nlohmann::json &j, const magic &magic) {
    j = nlohmann::json{{"name", magic.name},
                       {"magic_id", magic.id},
                       {"active", magic.active},
                       {"cd", magic.cd},
                       {"inner_damage", magic.inner_damage},
                       {"multiplier", magic.multiplier},
                       {"stats", magic.stats},
                       {"mp_cost", magic.mp_cost}};
}

void nibashared::from_json(const nlohmann::json &j, magic &magic) {
    j.at("name").get_to(magic.name);
    j.at("magic_id").get_to(magic.id);
    j.at("active").get_to(magic.active);
    j.at("cd").get_to(magic.cd);
    j.at("multiplier").get_to(magic.multiplier);
    j.at("inner_damage").get_to(magic.inner_damage);
    j.at("stats").get_to(magic.stats);
    j.at("mp_cost").get_to(magic.mp_cost);
}
