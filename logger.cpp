#include "logger.h"

namespace fs = boost::filesystem;

logger::logger(fs::path const &path) : ofs_{fs::ofstream{path}} {}

// [08:00:00][88.88.88.88][M:blka lvm kdlsfma sld]
void logger::log(
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> const& ssl_socket,
        std::string const &username,
        std::string const &message
) {
    std::string today = logger::get_time();
    std::string ip = ssl_socket.lowest_layer().remote_endpoint().address().to_string();

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