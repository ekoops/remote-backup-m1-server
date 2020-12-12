#include "logger.h"

namespace fs = boost::filesystem;

logger::logger(fs::path const &path) : ofs_{fs::ofstream{path}} {}

void logger::log(
        user const& usr,
        std::string const &message
) {
    std::string today = logger::get_time();
    std::string const& username = usr.username();
    std::string const& ip = usr.ip();

    std::ostringstream log;
    log << '[' << today << "]["
        << username << (username.empty() ? "" : "@") << ip << "]["
        << message << "]" << std::endl;
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