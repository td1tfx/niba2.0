#include "db_accessor.h"
#include <ozo/execute.h>
#include <ozo/request.h>
#include <ozo/shortcuts.h>
#include <algorithm>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <ozo/shortcuts.h>
#include <ozo/execute.h>
#include <ozo/request.h>

using namespace nibaserver;
using namespace ozo::literals;
namespace sev = boost::log::trivial;

constexpr std::size_t HASH_SIZE = 32;

db_accessor::db_accessor(const ozo::connector<ozo::connection_pool<ozo::connection_info<>>,
                                              ozo::connection_pool_timeouts> &conn) :
    conn_(conn) {
    logger_ = logger();
}

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
    ozo::rows_of<std::string, char, int, int, int, int> characters;
    auto conn = ozo::request(
        conn_,
        "SELECT name, gender, strength, dexterity, physique, spirit FROM player_character where id="_SQL +
            id,
        ozo::into(characters), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return {};
    }
    if (characters.size() == 0)
        return {};
    auto &[name, gender, strength, dexterity, physique, spirit] = characters.at(0);
    return nibashared::player{
        .name = name,
        .gender = gender,
        .attrs = {
            .strength = strength, .dexterity = dexterity, .physique = physique, .spirit = spirit}};
}

bool db_accessor::create_char(const std::string &id, const nibashared::player &player,
                              boost::asio::yield_context &yield) {
    ozo::error_code ec{};
    // the insert string is unfortunately long and hand written
    auto conn = ozo::execute(
        conn_,
        "INSERT INTO player_character(id, name, gender, strength, dexterity, physique, spirit) VALUES("_SQL +
            id + ","_SQL + player.name + ","_SQL + player.gender + ","_SQL + player.attrs.strength +
            ","_SQL + player.attrs.dexterity + ","_SQL + player.attrs.physique + ","_SQL +
            player.attrs.spirit + ")"_SQL,
        yield[ec]);

    // might not be an error
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }

    return true;
}
