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
add_executable(remote-backup-m1-server main.cpp core/server.cpp core/server.h core/connection.cpp core/connection.h core/request_handler.cpp core/request_handler.h utilities/tools.cpp utilities/tools.h communication/message.h communication/message.cpp communication/tlv_view.h communication/tlv_view.cpp directory/dir.h directory/dir.cpp directory/resource.h directory/resource.cpp core/user.cpp core/user.h communication/message_queue.cpp communication/message_queue.h utilities/logger.cpp utilities/logger.h communication/types.h core/open_streams.cpp core/open_streams.h)

target_link_libraries(remote-backup-m1-server ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
