#include <iostream>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <iomanip>
#include "ServerSocket.h"
#include "SocketBlockingQueue.h"
#include "Logger.h"
#include "RegexUtility.h"
#include "Configuration.h"

#define THREAD_NUM 8

bool authenticate() {

}

void f(SocketBlockingQueue & sbq) {
    while (1) {
        Socket socket = sbq.pop();
        //authentication
        if (authenticate()) {
            //socket.read();
        }
    }
}



int main(int argc, char const* const argv[]) {
    boost::regex e {"([a-z]{1,}):([a-z\\d]{1,})"};

    boost::filesystem::path config_file {argc < 2 ? "./config.cfg" : argv[1]};
    Configuration configuration {config_file};
//    configuration.get("ciao");

    SocketBlockingQueue sbq {};
    Logger logger {configuration.get("LOGGER_FILE_PATH")};

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
