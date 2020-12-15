#ifndef REMOTE_BACKUP_M1_SERVER_LOGGER_H
#define REMOTE_BACKUP_M1_SERVER_LOGGER_H

#include <fstream>
#include <netinet/in.h>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../core/user.h"
#include "../communication/message.h"
#include "../communication/message_queue.h"
#include "../communication/types.h"

/*
 * This class is used to log the communication result
 */
class logger : private boost::noncopyable {
    boost::filesystem::ofstream ofs_;
    std::unordered_map<communication::MSG_TYPE, std::string> msg_type_str_map_;
    std::unordered_map<communication::RESULT_TYPE, std::string> res_type_str_map_;
    static std::string get_time();

public:

    explicit logger(boost::filesystem::path const &path);

    void log(
            user const &usr,
            std::string const &message
    );

    void log(
            user const &usr,
            communication::MSG_TYPE msg_type,
            communication::RESULT_TYPE message_result,
            communication::RESULT_TYPE connection_result
    );


    ~logger() { this->ofs_.close(); }
};

#endif //REMOTE_BACKUP_M1_SERVER_LOGGER_H
