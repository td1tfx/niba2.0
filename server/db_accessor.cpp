#include "db_accessor.h"

#include <ozo/execute.h>
#include <ozo/query.h>
#include <ozo/request.h>
#include <ozo/shortcuts.h>
#include <ozo/transaction.h>

#include <algorithm>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace nibaserver;
using namespace ozo::literals;
namespace sev = boost::log::trivial;

constexpr std::size_t HASH_SIZE = 32;

db_accessor::db_accessor(const niba_ozo_connector &conn) : conn_(conn) { logger_ = logger(); }

bool db_accessor::login(const std::string &id, const std::string &password,
                        boost::asio::yield_context &yield) {
    ozo::error_code ec{};
    ozo::rows_of<std::string, ozo::pg::bytea, ozo::pg::bytea, bool> user_credential;
    auto conn = ozo::request(
        conn_, "SELECT id, hashed_password, salt, logged_in FROM user_id WHERE id="_SQL + id,
        ozo::into(user_credential), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }

    if (user_credential.size() == 0) {
        BOOST_LOG_SEV(logger_, sev::info) << "id does not exist " << id;
        // this username does not exist in the database
        return false;
    }

    auto &[username, hashed_password, salt, logged_in] = user_credential.at(0);
    const auto &raw_hashed_password = hashed_password.get();
    const auto &raw_salt = salt.get();

    unsigned char buffer[HASH_SIZE] = {0};
    memcpy(buffer, password.data(), (std::min)(HASH_SIZE, password.size()));
    for (std::size_t i = 0; i < HASH_SIZE; i++) {
        buffer[i] ^= raw_salt.at(i);
    }

    std::vector<char> digest(HASH_SIZE, 0);
    SHA256_CTX context;
    if (!SHA256_Init(&context))
        return false;

    if (!SHA256_Update(&context, (unsigned char *)buffer, HASH_SIZE))
        return false;

    if (!SHA256_Final((unsigned char *)&digest[0], &context))
        return false;

    if (memcmp((unsigned char *)&digest[0], (unsigned char *)&raw_hashed_password[0], HASH_SIZE) ==
        0) {
        if (logged_in) {
            BOOST_LOG_SEV(logger_, sev::info) << "user already logged in " << id;
            return false;
        }

        ozo::execute(conn_, "UPDATE user_id SET logged_in = true WHERE id="_SQL + id, yield[ec]);
        if (ec) {
            BOOST_LOG_SEV(db_accessor::logger_, sev::error)
                << ec.message() << " | " << ozo::error_message(conn) << " | "
                << ozo::get_error_context(conn);
            return false;
        }
        return true;
    }
    BOOST_LOG_SEV(logger_, sev::info) << "password mismatch " << id;
    return false;
}

bool db_accessor::logout(const std::string &id, boost::asio::yield_context &yield) {
    ozo::error_code ec{};
    auto conn =
        ozo::execute(conn_, "UPDATE user_id SET logged_in = false WHERE id="_SQL + id, yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }
    return true;
}

bool db_accessor::create_user(const std::string &id, const std::string &password,
                              boost::asio::yield_context &yield) {
    std::vector<char> salt(HASH_SIZE, 0);
    std::vector<char> hashed_password(HASH_SIZE, 0);
    if (RAND_bytes((unsigned char *)&salt[0], HASH_SIZE) != 1)
        return false;

    unsigned char buffer[HASH_SIZE] = {0};
    memcpy(buffer, password.data(), (std::min)(HASH_SIZE, password.size()));
    for (std::size_t i = 0; i < HASH_SIZE; i++) {
        buffer[i] ^= salt[i];
    }

    SHA256_CTX context;
    if (!SHA256_Init(&context))
        return false;

    if (!SHA256_Update(&context, (unsigned char *)buffer, HASH_SIZE))
        return false;

    if (!SHA256_Final((unsigned char *)&hashed_password[0], &context))
        return false;

    ozo::error_code ec{};
    ozo::pg::bytea salt_bytea(std::move(salt));
    ozo::pg::bytea pswd_bytea(std::move(hashed_password));

    auto conn = ozo::execute(conn_,
                             "INSERT INTO user_id (id, hashed_password, salt) VALUES ("_SQL + id +
                                 ","_SQL + pswd_bytea + ","_SQL + salt_bytea + ")"_SQL,
                             yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }

    return true;
}

// looks like this function 'broke' clang-format
std::optional<nibashared::player>
nibaserver::db_accessor::get_char(const std::string &id, boost::asio::yield_context &yield) {
    BOOST_LOG_SEV(logger_, sev::info) << "fetching character for " << id;
    ozo::error_code ec{};
    // not the best, we can probably do better
    // TODO: do something else, for example change how player is layed out
    ozo::rows_of<nibashared::player> characters;
    auto conn = ozo::request(conn_, "SELECT character FROM player_character WHERE id="_SQL + id,
                             ozo::into(characters), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return {};
    }
    if (characters.size() == 0)
        return {};
    return std::get<0>(characters.at(0));
}

std::tuple<std::vector<nibashared::magic>, std::vector<nibashared::equipment>, std::vector<int>>
nibaserver::db_accessor::get_aux(const std::string &name, boost::asio::yield_context &yield) {
    BOOST_LOG_SEV(logger_, sev::info) << "fetching character magic and equips for " << name;
    std::tuple<std::vector<nibashared::magic>, std::vector<nibashared::equipment>, std::vector<int>>
        ret;
    auto &[magics, equips, equipped_magic_ids] = ret;
    // auto &equips = ret.second;
    // ?? looking for ways to fix this
    ozo::rows_of<nibashared::magic> rows;
    ozo::error_code ec{};
    auto conn =
        ozo::request(conn_, "SELECT magic FROM player_magic WHERE character_name="_SQL + name,
                     ozo::into(rows), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return ret;
    }
    for (auto &r : rows) {
        magics.push_back(std::move(std::get<0>(r)));
    }
    BOOST_LOG_SEV(logger_, sev::info) << "getting player magics " << name;
    ozo::rows_of<std::vector<int>> equipped_magic_rows;
    conn = ozo::request(
        conn_, "SELECT magics FROM character_equipped_magic WHERE character_name = "_SQL + name,
        ozo::into(equipped_magic_rows), yield[ec]);
    if (equipped_magic_rows.size() != 0) {
        equipped_magic_ids = std::get<0>(equipped_magic_rows.back());
    }
    if (ec) {
        // not the best... we need better ways to return data
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return ret;
    }

    return ret;
    // skipping equipments for now...hoping there is a fix and we don't need to do this anymore
}

bool db_accessor::create_char(const std::string &id, const nibashared::player &player,
                              boost::asio::yield_context &yield) {
    ozo::error_code ec{};
    // the insert string is unfortunately long and hand written
    auto conn = ozo::execute(conn_,
                             "INSERT INTO player_character(id, character) VALUES("_SQL + id +
                                 ","_SQL + player + ")"_SQL,
                             yield[ec]);

    // might not be an error
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }
    return true;
}

bool db_accessor::create_magic(const std::string &player_name, const nibashared::magic &magic,
                               boost::asio::yield_context &yield) {
    ozo::error_code ec{};
    // note the binding is the name of the player_character
    auto query = "INSERT INTO player_magic(character_name, magic_id, magic) VALUES("_SQL +
                 player_name + ","_SQL + magic.magic_id + ","_SQL + magic + ")"_SQL;

    auto conn = ozo::execute(conn_, query, yield[ec]);

    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }
    return true;
}

bool db_accessor::fuse_magic(const std::string &player_name, const nibashared::magic &magic,
                             int delete_magic_id, const std::vector<int> &equipped_magic_ids,
                             boost::asio::yield_context &yield) {
    ozo::error_code ec{};

    auto transaction = ozo::begin(conn_, yield);
    ozo::result result;
    auto query = "UPDATE player_magic SET magic = "_SQL + magic + " WHERE character_name = "_SQL +
                 player_name + " AND "_SQL + " magic_id = "_SQL + magic.magic_id;
    ozo::request(transaction, query, std::ref(result), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << "111";
        return false;
    }
    ozo::request(transaction,
                 "DELETE FROM player_magic WHERE character_name = "_SQL + player_name +
                     " AND "_SQL + "magic_id = "_SQL + delete_magic_id,
                 std::ref(result), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << "222";
        return false;
    }
    ozo::request(transaction,
                 "INSERT INTO character_equipped_magic(character_name, magics) VALUES("_SQL +
                     player_name + ","_SQL + equipped_magic_ids +
                     ") ON CONFLICT (character_name) DO UPDATE SET magics ="_SQL +
                     equipped_magic_ids,
                 std::ref(result), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << "333";
        return false;
    }
    auto conn = ozo::commit(std::move(transaction), yield[ec]);

    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }
    return true;
}

bool db_accessor::equip_magics(const std::string &player_name,
                               const std::vector<int> &equipped_magic_ids,
                               boost::asio::yield_context &yield) {
    ozo::error_code ec{};
    auto conn = ozo::execute(
        conn_,
        "INSERT INTO character_equipped_magic(character_name, magics) VALUES("_SQL + player_name +
            ","_SQL + equipped_magic_ids +
            ") ON CONFLICT (character_name) DO UPDATE SET magics ="_SQL + equipped_magic_ids,
        yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }
    return true;
}