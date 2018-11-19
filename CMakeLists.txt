project(niba)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_COMPILER $ENV{CXX})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(Boost_USE_STATIC_LIBS ON)
# TODO flag this
set(CMAKE_BUILD_TYPE Debug)

add_compile_definitions(BOOST_COROUTINES_NO_DEPRECATION_WARNING)

find_package(Boost 1.68.0 REQUIRED COMPONENTS system context coroutine log regex)
find_package(nlohmann_json 3.3.0 REQUIRED)
find_package(OpenSSL 1.1.0 REQUIRED)
find_package(Threads REQUIRED)

file(GLOB SHARED_SRC_LIST shared/*.cpp)
file(GLOB SERVER_SRC_LIST server/*.cpp)
file(GLOB CLIENT_SRC_LIST client/*.cpp)

include_directories(shared/)

add_executable(niba-server ${SERVER_SRC_LIST} ${SHARED_SRC_LIST})
add_executable(niba-client ${CLIENT_SRC_LIST} ${SHARED_SRC_LIST})

target_link_libraries(niba-server
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    Boost::system
    Boost::context
    Boost::coroutine
    Boost::log
    Boost::regex
    Threads::Threads
    nlohmann_json::nlohmann_json)

target_link_libraries(niba-client
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    Boost::system
    Boost::context
    Boost::coroutine
    Boost::log
    Boost::regex
    Threads::Threads 
    nlohmann_json::nlohmann_json)

configure_file(staticdata/character.json character.json COPYONLY)
configure_file(staticdata/magic.json magic.json COPYONLY)
