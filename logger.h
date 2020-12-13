#ifndef REMOTE_BACKUP_M1_SERVER_LOGGER_H
#define REMOTE_BACKUP_M1_SERVER_LOGGER_H

#include <fstream>
#include <netinet/in.h>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "user.h"
#include "message.h"
#include "message_queue.h"

enum RESULT_TYPE {
    OK,
    ERROR,
    NONE
};

class logger : private boost::noncopyable {
    boost::filesystem::ofstream ofs_;
    std::map<RESULT_TYPE, std::string> result_type_str_map_;
    std::map<communication::MESSAGE_TYPE, std::string> msg_type_str_map_;

    static std::string get_time();

public:

    logger(boost::filesystem::path const &path);

    void log(
            user const &usr,
            std::string const &message
    );

    void log(
            user const &usr,
            communication::MESSAGE_TYPE msg_type,
            RESULT_TYPE message_result,
            RESULT_TYPE connection_result
    );


    ~logger() { this->ofs_.close(); }
};

#endif //REMOTE_BACKUP_M1_SERVER_LOGGER_H
