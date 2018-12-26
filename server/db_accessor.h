#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "structs.h"
#include "logger.h"
#include <boost/asio/spawn.hpp>
#include <ozo/request.h>
#include <ozo/connection_info.h>
#include <ozo/connection_pool.h>
#include <ozo/shortcuts.h>
#include <ozo/execute.h>
#include <ozo/connection.h>
//#include <ozo/type_traits.h>

//OZO_PG_DEFINE_TYPE_AND_ARRAY(std::vector<char>, "bytea", BYTEAOID, 1001, ozo::dynamic_size)

namespace nibaserver {

class db_accessor {
public:
    db_accessor(const ozo::connector<ozo::connection_pool<ozo::connection_info<>>, ozo::connection_pool_timeouts> &conn);
    ~db_accessor() = default;
    bool login(boost::asio::yield_context &yield,
        const std::string &id, const std::string &password);

    bool logout(boost::asio::yield_context &yield, const std::string &id);
    
    bool create_user(boost::asio::yield_context &yield,
        const std::string &id, const std::string &password);

    // NOTE no save char functions yet
    std::optional<nibashared::character> get_char(const std::string &id);
    bool create_char(const std::string &id, nibashared::character&& character);

private:
    // TODO: add mutex, but whatever, probably have a db running before that
    logger logger_;
    std::unordered_map<std::string, nibashared::character> char_tbl_;
    const ozo::connector<ozo::connection_pool<ozo::connection_info<>>, ozo::connection_pool_timeouts> &conn_;
};

} // namespace nibaserver
