#include "logger.h"

namespace fs = boost::filesystem;
using namespace communication;

/**
 * Create a logger instance with a given log file path
 *
 * @param path the path on which the created logger has to log
 * @return void
 */
logger::logger(fs::path const &path) : ofs_{fs::ofstream{path, std::ios_base::app}} {
    this->msg_type_str_map_ = {
            {NONE,       "-"},
            {CREATE,     "CREATE"},
            {UPDATE,     "UPDATE"},
            {ERASE,      "ERASE"},
            {SYNC,       "SYNC"},
            {AUTH,       "AUTH"},
            {KEEP_ALIVE, "KEEP_ALIVE"}
    };

    this->res_type_str_map_ = {
            {RES_OK, "OK"},
            {RES_ERR, "ERR"},
            {RES_NONE, "-"}
    };
}

/**
 * Log the given message related to the given user
 *
 * @param usr the user to whom the message refers
 * @param message the message that has to be logged
 * @return void
 */
void logger::log(user const &usr, std::string const &message) {
    std::string today = logger::get_time();
    std::string const &username = usr.username();
    std::string const &ip = usr.ip();

    std::ostringstream log;
    log << '[' << today << "]["
        << username << (username.empty() ? "" : "@") << ip << "]["
        << message << ']' << std::endl;
    this->ofs_ << log.str();
}

/**
 * Log the result of a communication instance for the given user
 *
 * @param usr the user to whom the message refers
 * @param msg_type the message type of the communication instance
 * @param message_result the message processing result
 * @param connection_result the state of connection during communication
 * @return void
 */
void logger::log(
        user const &usr,
        MSG_TYPE msg_type,
        RESULT_TYPE message_result,
        RESULT_TYPE connection_result
) {
    std::string today = logger::get_time();
    std::string const &username = usr.username();
    std::string const &ip = usr.ip();
    auto msg_type_str = this->msg_type_str_map_.find(msg_type)->second;
    auto msg_res_str = this->res_type_str_map_.find(message_result)->second;
    auto con_res_str = this->res_type_str_map_.find(connection_result)->second;
    std::ostringstream log;
    log << '[' << today << "]["
        << username << (username.empty() ? "" : "@") << ip << "]["
        << "TYPE: " << msg_type_str
        << " RES: " << msg_res_str
        << " CON: " << con_res_str << ']' << std::endl;
    this->ofs_ << log.str();
}

/*
 * An internal logger method used to obtain the current time
 * in a string format
 *
 * @return a string representation of the current time
 */
std::string logger::get_time() {
    using namespace boost::posix_time;
    ptime today{second_clock::universal_time()};
    std::string today_string{to_iso_extended_string(today)};
    std::istringstream iss{today_string};
    std::string today_substring;
    std::getline(iss, today_substring, ',');
    return today_substring;
}