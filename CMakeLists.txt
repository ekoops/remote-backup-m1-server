cmake_minimum_required(VERSION 3.17)
project(remote_backup_m1_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -pthread)
#[[set(Boost_USE_STATIC_LIBS ON)]]
#[[set(OPENSSL_USE_STATIC_LIBS TRUE)]]

set(BOOST_ROOT boost)
find_package(Boost 1.73.0 REQUIRED COMPONENTS filesystem date_time regex)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
add_executable(remote_backup_m1_server main.cpp SocketBlockingQueue.cpp SocketBlockingQueue.h Logger.cpp Logger.h Configuration.cpp Configuration.h RegexUtility.cpp RegexUtility.h)

target_link_libraries(remote_backup_m1_server ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)

