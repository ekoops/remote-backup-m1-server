cmake_minimum_required(VERSION 3.17)
project(remote-backup-m1-server)

set(CMAKE_CXX_STANDARD 20)
#[[set(Boost_USE_STATIC_LIBS ON)]]
#[[set(OPENSSL_USE_STATIC_LIBS TRUE)]]

set(BOOST_ROOT boost)
set(CMAKE_CXX_FLAGS -pthread)
find_package(Boost 1.73.0 REQUIRED COMPONENTS filesystem thread serialization regex program_options)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
add_executable(remote-backup-m1-server main.cpp server.cpp server.h connection.cpp connection.h request_handler.cpp request_handler.h tools.cpp tools.h message.h message.cpp tlv_view.h tlv_view.cpp dir.h dir.cpp resource.h resource.cpp user.cpp user.h message_queue.cpp message_queue.h)

target_link_libraries(remote-backup-m1-server ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
