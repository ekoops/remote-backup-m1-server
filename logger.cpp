#include "logger.h"

namespace fs = boost::filesystem;
using namespace communication;

logger::logger(fs::path const &path) : ofs_{fs::ofstream{path, std::ios_base::app}} {
    this->result_type_str_map_.insert({
                                         {RESULT_TYPE::OK,               "OK"},
                                         {RESULT_TYPE::ERROR,            "ERROR"},
                                         {RESULT_TYPE::NONE,"-"}
                                 });
    this->msg_type_str_map_.insert({
                                          {MESSAGE_TYPE::NONE, "-"},
                                          {MESSAGE_TYPE::SYNC, "SYNC"},
                                          {MESSAGE_TYPE::AUTH, "AUTH"},
                                          {MESSAGE_TYPE::CREATE, "CREATE"},
                                          {MESSAGE_TYPE::UPDATE, "UPDATE"},
                                          {MESSAGE_TYPE::ERASE, "ERASE"},
                                          {MESSAGE_TYPE::KEEP_ALIVE, "KEEP_ALIVE"}
    });
}

void logger::log(
        user const &usr,
        std::string const &message
) {
    std::string today = logger::get_time();
    std::string const &username = usr.username();
    std::string const &ip = usr.ip();

    std::ostringstream log;
    log << '[' << today << "]["
        << username << (username.empty() ? "" : "@") << ip << "]["
        << message << ']' << std::endl;
    this->ofs_ << log.str();
}

void logger::log(
        user const &usr,
        MESSAGE_TYPE msg_type,
        RESULT_TYPE message_result,
        RESULT_TYPE connection_result
) {
    std::string today = logger::get_time();
    std::string const &username = usr.username();
    std::string const &ip = usr.ip();
    auto msg_type_str = this->msg_type_str_map_.find(msg_type)->second;
    auto msg_res_str = this->result_type_str_map_.find(message_result)->second;
    auto con_res_str = this->result_type_str_map_.find(connection_result)->second;
    std::ostringstream log;
    log << '[' << today << "]["
        << username << (username.empty() ? "" : "@") << ip << "]["
        << "TYPE: " << msg_type_str
        << " RES: " << msg_res_str
        << " CON: " << con_res_str << ']' << std::endl;
    this->ofs_ << log.str();
}

std::string logger::get_time() {
    using namespace boost::posix_time;
    ptime today{second_clock::universal_time()};
    std::string today_string{to_iso_extended_string(today)};
    std::istringstream iss{today_string};
    std::string today_substring;
    std::getline(iss, today_substring, ',');
    return today_substring;
}