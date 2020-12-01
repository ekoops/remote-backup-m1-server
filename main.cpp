//
// Created by leonardo on 01/12/20.
//

#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "server.h"

namespace fs = boost::filesystem;

int main(int argc, char *argv[]) {

    if (argc != 5) {
        std::cerr << "Usage: ./server <address> <port> <threads> <backup_root>" << std::endl;
        return 1;
    }
    try {
        std::string server_addr{argv[1]};
        std::string port{argv[2]};
        std::uint16_t num_threads = boost::lexical_cast<std::uint16_t>(argv[3]);
        boost::filesystem::path backup_root{boost::filesystem::canonical(argv[4])};
        if (!fs::is_directory(backup_root)) {
            std::cerr << "<backup_root> provided is not a directory" << std::endl;
            return 2;
        }

        // Initialise the server.
        server s{server_addr, port, num_threads, backup_root};

        // Run the server until stopped.
        s.run();
    }
    catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}