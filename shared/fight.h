#pragma once
#include "global_defs.h"
#include "rng.h"
#include "structs.h"
#include <algorithm>
#include <array>
#include <boost/assert.hpp>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

namespace nibashared {

struct magic_ex {
    int cd{0};
    std::optional<magic> real_magic{};

    void heatup() {
        if (real_magic.has_value()) {
            cd = real_magic.value().cd;
        }
    }
    void cooldown(int ticks) { cd = std::max(cd - ticks, 0); }
};

constexpr int FIGHT_MAX_PROG = 1000;
constexpr int DEFENCE_EXTENSION = 100;
constexpr int INNER_BASE = 100;

using fightable_magics = std::array<magic_ex, MAX_ACTIVE_MAGIC>;

// composition over inheritance
class fightable {
public:
    battlestats stats;
    int idx;  // idx within all_
    int team; // takes value 0 or 1
    int progress{0};

    // Note here the person may not have full 5 magics
    fightable_magics magics;

    int ticks_calc() const {
        if (progress >= FIGHT_MAX_PROG)
            return 0;
        // this rounds up
        // for example, the current progress is 649, speed is 35
        // (1000 - 649 + 35 - 1) / 35 = 11 ticks to finish the 351 progress
        // say the progress is now 651, speed is still 35
        // (1000 - 651 + 35 - 1) / 35 = 10 ticks to finish the 349 progress
        return (FIGHT_MAX_PROG - progress + stats.speed - 1) / stats.speed;
    }

    // TODO: make this rng templated
    template<typename RNG>
    int damage_calc(const magic &chosen_magic, const fightable &defender, RNG &rng) const {
        int multiplier = chosen_magic.multiplier;
        int accuracy =
            static_cast<int>(stats.accuracy * (100.0 / (stats.accuracy + defender.stats.accuracy)));
        if (accuracy <= rng(0, 99))
            return 0;
        // TODO: Niba, finish this
        int inner_damage = chosen_magic.inner_damage;
        auto physical_damage_reduction =
            defender.stats.defence * 1.0 / (defender.stats.defence + DEFENCE_EXTENSION);
        auto inner_damage_multiplier = 1.0 + stats.inner_power / INNER_BASE;

        return static_cast<int>((rng(stats.attack_min, stats.attack_max) / 100.0 * multiplier) *
                                    (1.0 - physical_damage_reduction) +
                                inner_damage_multiplier * inner_damage);
    }

    int threat_calc() const { return stats.attack_min + stats.attack_max; }

    // note that return also makes sure the real magic is there if has value
    std::optional<std::size_t> pick_magic_idx() {
        // potential optimization: precompute the sequence of magics
        for (std::size_t i = 0; i < magics.size(); i++) {
            if (magics[i].real_magic.has_value() && magics[i].cd == 0) {
                return i;
            }
        }
        return {};
    }
};

class fight {
public:
    // the return is the RNG results! this is to sync with the client
    // 1. it's hard to set random seed to get the same randoms for different platform of clients
    // 2. so instead of returning damage numbers, returning randoms
    fight(std::vector<fightable> &&friends, std::vector<fightable> &&enemies) {
        friends_ = friends.size();
        enemies_ = enemies.size();
        std::for_each(friends.begin(), friends.end(), [](auto &f) { f.team = 0; });
        std::for_each(enemies.begin(), enemies.end(), [](auto &f) { f.team = 1; });
        std::move(friends.begin(), friends.end(), std::back_inserter(all_));
        std::move(enemies.begin(), enemies.end(), std::back_inserter(all_));
        for (std::size_t i = 0; i < all_.size(); i++) {
            all_.at(i).idx = i;
            for (auto &m : all_.at(i).magics) {
                if (m.real_magic) {
                    all_.at(i).stats += (*m.real_magic).stats;
                }
            }
        }
        // sort by speed, use the 'arbitrary' idx to break tie
        std::sort(all_.begin(), all_.end(), [](const auto &lhs, const auto &rhs) {
            return std::tie(lhs.stats.speed, lhs.idx) < std::tie(rhs.stats.speed, rhs.idx);
        });
        for (std::size_t i = 0; i < all_.size(); i++) {
            all_.at(i).idx = i;
        }
    }

    // RNG will be used to provide random numbers (from 0-1 likely)
    // the test client will have the set of numbers received from server
    template<typename RNG>
    int go(RNG &rng) {

        std::size_t team_alive_count[]{friends_, enemies_};
        std::vector<std::size_t> alive;
        alive.reserve(all_.size());
        for (;;) {
            if (team_alive_count[0] == 0 || team_alive_count[1] == 0)
                break;
            // all alive indices
            for (std::size_t i = 0; i < all_.size(); i++) {
                if (all_.at(i).stats.hp <= 0) {
                    continue;
                }
                alive.push_back(i);
            }

            using tick_idx_p = std::pair<int, std::size_t>;
            // find the person that is the fastest to act
            tick_idx_p min_tick{all_.at(alive[0]).ticks_calc(), alive[0]};
            // figure out the ticks to make it to the next action
            for (std::size_t i = 1; i < alive.size(); i++) {
                tick_idx_p tick{all_.at(alive[i]).ticks_calc(), alive[i]};
                if (tick < min_tick)
                    min_tick = tick;
            }

            auto &[ticks, attacker_idx] = min_tick;
            auto &attacker = all_.at(attacker_idx);

            // make everyone progress and find the target, to break ties, again use idx
            using threat_idx_p = std::pair<int, std::size_t>;
            // it's unknown which idx is the first enemy
            // we use (threat=-1, idx=attacker) as a placeholder
            threat_idx_p max_threat{-1, attacker.idx};
            for (auto idx : alive) {
                auto &p = all_.at(idx);
                p.progress += p.stats.speed * ticks;
                std::for_each(p.magics.begin(), p.magics.end(),
                              [ticks](auto &mex) { mex.cooldown(ticks); });
                // maybe optimization: maintain a priority list (heap) of top threats
                if (p.team != attacker.team) {
                    threat_idx_p threat{p.threat_calc(), idx};
                    if (threat > max_threat)
                        max_threat = threat;
                }
            }
            BOOST_ASSERT(max_threat.second != attacker.idx);
            auto &defender = all_.at(max_threat.second);

            // now pick a magic to use
            auto chosen_magic_idx = attacker.pick_magic_idx();

            // use just puts it on cd, extra + 1
            auto dmg = 0;
            if (chosen_magic_idx) {
                auto &chosen_magicex = attacker.magics[(*chosen_magic_idx)];
                chosen_magicex.heatup();
                dmg = attacker.damage_calc(*(chosen_magicex.real_magic), defender, rng);
            } else {
                dmg = attacker.damage_calc(DEFAULT_MAGIC, defender, rng);
            }

            defender.stats.hp -= dmg;
            std::cout << attacker.idx << " dealt " << dmg << " on " << defender.idx << std::endl;

            if (defender.stats.hp <= 0) {
                team_alive_count[defender.team] -= 1;
            }
            attacker.progress = 0;
            alive.clear();
        }
        std::cout << team_alive_count[0] << " " << team_alive_count[1] << std::endl;
        return team_alive_count[1] == 0 ? 0 : 1;
    }

private:
    std::size_t friends_;
    std::size_t enemies_;
    std::vector<fightable> all_;
};

} // namespace nibashared