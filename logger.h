#ifndef REMOTE_BACKUP_M1_SERVER_LOGGER_H
#define REMOTE_BACKUP_M1_SERVER_LOGGER_H

#include <fstream>
#include <netinet/in.h>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "user.h"

class logger : private boost::noncopyable {
    boost::filesystem::ofstream ofs_;

    static std::string get_time();

public:
    logger(boost::filesystem::path const &path);

    void log(
            user const& usr,
            std::string const &message
    );


    ~logger() { this->ofs_.close(); }
};

#endif //REMOTE_BACKUP_M1_SERVER_LOGGER_H
