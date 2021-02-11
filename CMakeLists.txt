project(niba)

cmake_minimum_required(VERSION 3.12)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_COMPILER $ENV{CXX})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Wunreachable-code -Wuninitialized")

# https://stackoverflow.com/questions/28532444/cmake-string-token-inclusion-check
if (CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-string-literal-operator-template")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(Boost_USE_STATIC_LIBS ON)

add_compile_definitions(BOOST_COROUTINES_NO_DEPRECATION_WARNING)
# TODO: enable only for debug builds
add_compile_definitions(DELAYFACTOR=0.1)

SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.70.0 REQUIRED COMPONENTS context coroutine log log_setup regex system)

find_package(nlohmann_json 3.3.0 REQUIRED)
find_package(OpenSSL 1.1.0 REQUIRED)
find_package(resource_pool REQUIRED) # resource_pool must be found before ozo
find_package(ozo REQUIRED)
find_package(PostgreSQL 10 REQUIRED)
find_package(Threads REQUIRED)

file(GLOB SHARED_SRC_LIST shared/*.cpp)
file(GLOB SERVER_SRC_LIST server/*.cpp)
file(GLOB CLIENT_SRC_LIST client/*.cpp)

include_directories(shared/)

add_executable(niba-server ${SERVER_SRC_LIST} ${SHARED_SRC_LIST})
add_executable(niba-client ${CLIENT_SRC_LIST} ${SHARED_SRC_LIST})

target_compile_definitions(niba-client PUBLIC NIBACLIENT)

set(LINK_LIBS 
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    Boost::context
    Boost::coroutine
    Boost::log
    Boost::log_setup
    Boost::regex
    Boost::system
    nlohmann_json::nlohmann_json
    Threads::Threads)

target_link_libraries(niba-server ${LINK_LIBS} elsid::resource_pool ozo ${PostgreSQL_LIBRARIES})
target_link_libraries(niba-client ${LINK_LIBS})
