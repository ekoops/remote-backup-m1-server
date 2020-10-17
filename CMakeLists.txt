cmake_minimum_required(VERSION 3.17)
project(remote_backup_m1_server)

set(CMAKE_CXX_STANDARD 20)
#[[set(Boost_USE_STATIC_LIBS ON)]]
#[[set(OPENSSL_USE_STATIC_LIBS TRUE)]]

set(BOOST_ROOT boost)
find_package(Boost 1.73.0 REQUIRED COMPONENTS filesystem date_time)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
add_executable(remote_backup_m1_server main.cpp SocketBlockingQueue.cpp SocketBlockingQueue.h Logger.cpp Logger.h)

target_link_libraries(remote_backup_m1_server ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)

