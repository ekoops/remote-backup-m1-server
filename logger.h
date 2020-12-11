#ifndef REMOTE_BACKUP_M1_SERVER_LOGGER_H
#define REMOTE_BACKUP_M1_SERVER_LOGGER_H

#include <fstream>
#include <netinet/in.h>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class logger : private boost::noncopyable {
    boost::filesystem::ofstream ofs_;

    static std::string get_time();

public:
    logger(boost::filesystem::path const &path);

    void log(
            boost::asio::ssl::stream<boost::asio::ip::tcp::socket> const& ssl_socket,
            std::string const &username,
            std::string const &message
    );


    ~logger() { this->ofs_.close(); }
};

#endif //REMOTE_BACKUP_M1_SERVER_LOGGER_H
