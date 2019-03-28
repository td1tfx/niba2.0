#include "data_init.h"
#include "connector.h"
#include "gamedata.h"

#include <boost/asio/spawn.hpp>
#include <ozo/execute.h>
#include <ozo/request.h>
#include <ozo/shortcuts.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <string_view>

namespace nibaserver {

template<typename T, typename Query, typename Connector>
void init_one(Query q, const Connector &connector, const std::string &id_key,
              ozo::io_context &ioc) {
    boost::asio::spawn(ioc, [q, connector, id_key](boost::asio::yield_context yield) {
        ozo::rows_of<std::string> data_row;
        ozo::request(connector, q, ozo::into(data_row), yield);
        auto data_str = std::get<0>(data_row.at(0));
        nibashared::staticdata::internal_map<T> datamap;
        try {
            std::cout << data_str << std::endl;
            nlohmann::json serialized_data = nlohmann::json::parse(data_str);
            for (auto &element : serialized_data) {
                datamap[element[id_key]] = element;
            }
        } catch (std::exception &e) {
            // I don't care at this point, this should be tested statically
            std::cout << "failed " << e.what() << std::endl;
            exit(-1);
        }
        nibashared::staticdata::init(std::move(datamap));
    });
}

void init_gamedata(const std::string &static_conn_str) {
    using namespace nibashared;
    using namespace ozo::literals;
    // read from postgres here
    const auto connection_info = ozo::make_connection_info(static_conn_str);
    ozo::io_context ioc;
    const auto connector = ozo::make_connector(connection_info, ioc);

    init_one<character>("SELECT to_json::TEXT FROM character_dump"_SQL, connector, "character_id",
                        ioc);
    init_one<magic>("SELECT to_json::TEXT FROM magic_dump"_SQL, connector, "magic_id", ioc);
    init_one<equipment>("SELECT to_json::TEXT FROM item_dump"_SQL, connector, "equipment_id", ioc);
    init_one<map>("SELECT to_json::TEXT FROM map_dump"_SQL, connector, "map_id", ioc);
    ioc.run();
}

} // namespace nibaserver