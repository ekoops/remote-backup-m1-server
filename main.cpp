#include <iostream>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl/impl/src.hpp>
#include "ServerSocket.h"
#include "SocketBlockingQueue.h"
#include "Logger.h"

#define THREAD_NUM 8

void f(SocketBlockingQueue & sbq) {
    while (1) {
        Socket socket = sbq.pop();
        socket.read();
    }
}

std::pair<std::string, std::string> line_to_pair(std::string const& line) {
    std::istringstream iss {line};
    std::string key;
    std::getline(iss, key, ':');
    std::string value;
    std::getline(iss, value);
    return {key, value};
}

void get_conf(boost::filesystem::path const& config_file) {
    if (!boost::filesystem::exists(config_file)) {
        throw std::runtime_error {"Configuration not found"};
    }
    std::map<std::string, std::string> map;

    std::ifstream ifs {config_file};
    std::string line;
    while (std::getline(ifs, line)) {
        std::pair<std::string, std::string> pair = line_to_pair(line);
    }
    ifs.close();
}

//
int main(int argc, char const* const argv[]) {
    boost::filesystem::path config_file {argc < 2 ? "./config.cfg" : argv[1]};

    SocketBlockingQueue sbq {};
    Logger logger {};
    std::vector<std::thread> threads;
    threads.reserve(THREAD_NUM);
    for (int i=0; i<THREAD_NUM; i++) {
        threads.emplace_back(f, std::ref(sbq));
    }

    ServerSocket ss{5000};
    while (1) {
        struct sockaddr_in client_sockaddr_in;
        unsigned int client_sockaddr_in_len;
        Socket client_socket = ss.accept(&client_sockaddr_in, &client_sockaddr_in_len);
        logger.log("Accepted socket", &client_sockaddr_in);
        sbq.push(std::move(client_socket));
    }


    return 0;
}
