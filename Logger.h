//
// Created by leonardo on 17/10/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_LOGGER_H
#define REMOTE_BACKUP_M1_SERVER_LOGGER_H
#include <fstream>
#include <netinet/in.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <arpa/inet.h>

class Logger {
    std::ofstream ofs;
    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;
public:
    Logger(boost::filesystem::path const& path): ofs {std::ofstream {path}} {}
    void log(std::string const& message, struct sockaddr_in* sockaddr_in) {
        using namespace boost::posix_time;
        ptime today {second_clock::universal_time()};
        std::string today_string {to_iso_extended_string(today)};
        std::istringstream iss {today_string};
        std::string today_substring;
        std::getline(iss, today_substring, ',');

        std::ostringstream log;
        char ip[16];
        int port = ntohs(sockaddr_in->sin_port);
        inet_ntop(AF_INET, &sockaddr_in->sin_addr, ip, sizeof(ip));
        log << '[' << today_substring << "][" << ip << ':' << port << "][M:" << message << '\n';
        ofs << log.str();
    }

    ~Logger() {this->ofs.close();}
};


#endif //REMOTE_BACKUP_M1_SERVER_LOGGER_H
