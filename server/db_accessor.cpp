#include "db_accessor.h"

#include <ozo/shortcuts.h>
#include <ozo/execute.h>
#include <ozo/request.h>

#include <algorithm>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace nibaserver;
using namespace ozo::literals;
namespace sev = boost::log::trivial;

constexpr std::size_t HASH_SIZE = 32;

db_accessor::db_accessor(const ozo::connector<ozo::connection_pool<ozo::connection_info<>>,
                                              ozo::connection_pool_timeouts> &conn) :
    conn_(conn) {
    logger_ = logger();
}

bool db_accessor::login(boost::asio::yield_context &yield, const std::string &id,
                        const std::string &password) {
    ozo::error_code ec{};
    ozo::rows_of<ozo::pg::name, ozo::pg::bytea, ozo::pg::bytea, bool> user_credential;
    auto conn = ozo::request(
        conn_,
        "SELECT username, hashed_password, salt, logged_in FROM user_id WHERE username="_SQL + id,
        ozo::into(user_credential), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }

    if (user_credential.size() == 0) {
        BOOST_LOG_SEV(logger_, sev::info) << "username does not exist " << id;
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

        ozo::execute(conn_, "UPDATE user_id SET logged_in = true WHERE username="_SQL + id,
                     yield[ec]);
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

bool db_accessor::logout(boost::asio::yield_context &yield, const std::string &id) {
    ozo::error_code ec{};
    auto conn = ozo::execute(conn_, "UPDATE user_id SET logged_in = false WHERE username="_SQL + id,
                             yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }
    return true;
}

bool db_accessor::create_user(boost::asio::yield_context &yield, const std::string &id,
                              const std::string &password) {
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
                             "INSERT INTO user_id (username, hashed_password, salt) VALUES ("_SQL +
                                 id + ","_SQL + pswd_bytea + ","_SQL + salt_bytea + ")"_SQL,
                             yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                                           << " | " << ozo::get_error_context(conn);
        return false;
    }

    return true;
}

std::optional<nibashared::character> nibaserver::db_accessor::get_char(const std::string &id) {
    auto iter = char_tbl_.find(id);
    if (iter == char_tbl_.end())
        return {};
    return iter->second;
}

bool db_accessor::create_char(const std::string &id, nibashared::character &&character) {
    auto c = get_char(id);
    if (c) {
        return false;
    }
    char_tbl_[id] = std::move(character);

    return true;
}
