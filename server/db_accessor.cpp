#include "db_accessor.h"
#include <algorithm>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace nibaserver;
using namespace ozo::literals;
namespace sev = boost::log::trivial;

std::unordered_map<std::string, nibashared::character> db_accessor::char_tbl_;

constexpr std::size_t HASH_SIZE = 32;
static auto conn_info = ozo::make_connection_info("host=127.0.0.1 port=5432 dbname=niba");
nibaserver::logger db_accessor::logger_ = logger();

bool nibaserver::db_accessor::login(boost::asio::io_context &ioc, boost::asio::yield_context &yield,
    const std::string &id, const std::string &password) {
    ozo::error_code ec{};
    ozo::rows_of<ozo::pg::name, std::vector<char>, std::vector<char>, bool> user_credential;
    auto conn = ozo::request(ozo::make_connector(conn_info, ioc),
                             "SELECT username, hashed_password, salt, logged_in FROM user_id WHERE username="_SQL + id,
                             ozo::into(user_credential), yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(db_accessor::logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
            << " | " << ozo::get_error_context(conn);
        return false;
    }

    if (user_credential.size() == 0) {
        // this username does not exist in the database
        return false;
    }

    auto &[username, hashed_password, salt, logged_in] = user_credential.at(0);


    unsigned char buffer[HASH_SIZE] = {0};
    memcpy(buffer, password.data(), (std::min)(HASH_SIZE, password.size()));
    for (std::size_t i = 0; i < HASH_SIZE; i++) {
        buffer[i] ^= salt[i];
    }

    std::vector<char> digest(HASH_SIZE, 0);
    SHA256_CTX context;
    if (!SHA256_Init(&context))
        return false;

    if (!SHA256_Update(&context, (unsigned char *)buffer, HASH_SIZE))
        return false;

    if (!SHA256_Final((unsigned char *)&digest[0], &context))
        return false;

    if (memcmp((unsigned char *)&digest[0], (unsigned char *)&hashed_password[0], HASH_SIZE) == 0) {
        if (logged_in)
            return false;

        ozo::execute(conn, "UPDATE user_id SET logged_in = true WHERE username="_SQL + id , yield[ec]);
        if (ec) {
            BOOST_LOG_SEV(db_accessor::logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
                << " | " << ozo::get_error_context(conn);
            return false;
        }
        return true;
    }
    return false;
}

bool nibaserver::db_accessor::logout(boost::asio::io_context &ioc, boost::asio::yield_context &yield, const std::string &id) {
    ozo::error_code ec{};
    auto conn = ozo::execute(ozo::make_connector(conn_info, ioc),
        "UPDATE user_id SET logged_in = false WHERE username="_SQL + id , yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(db_accessor::logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
            << " | " << ozo::get_error_context(conn);
        return false;
    }
    return true;
}

bool nibaserver::db_accessor::create_user(boost::asio::io_context &ioc, boost::asio::yield_context &yield, 
    const std::string &id, const std::string &password) {
    std::vector<char> salt(32, 0);
    std::vector<char> hashed_password(32, 0);
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
    auto conn = ozo::execute(ozo::make_connector(conn_info, ioc),
        "INSERT INTO user_id (username, hashed_password, salt) VALUES ("_SQL
            + id + ","_SQL + hashed_password + ","_SQL + salt + ")"_SQL, yield[ec]);
    if (ec) {
        BOOST_LOG_SEV(db_accessor::logger_, sev::error) << ec.message() << " | " << ozo::error_message(conn)
            << " | " << ozo::get_error_context(conn);
        return false;
    }

    return true;
}

std::optional<nibashared::character> nibaserver::db_accessor::get_char(const std::string & id)
{
    auto iter = char_tbl_.find(id);
    if (iter == char_tbl_.end()) return {};
    return iter->second;
}

bool nibaserver::db_accessor::create_char(const std::string & id, nibashared::character && character)
{
    // this is bad if we have a mutex
    auto c = get_char(id);
    if (c) {
        return false;
    }
    char_tbl_[id] = std::move(character);

    return true;
}
