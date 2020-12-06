//
// Created by leonardo on 01/12/20.
//

#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "server.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

po::variables_map parse_options(int argc, char const *const argv[]) {
    try {
        po::options_description desc("Backup server options");
        desc.add_options()
                ("help,h",
                 "produce help message")
                ("address,A",
                 po::value<std::string>()->required(),
                 "set backup server address")
                ("service,S",
                 po::value<std::string>()->required(),
                 "set backup server service name/port number")
                ("backup-root,R",
                 po::value<fs::path>()->default_value(
                         fs::path{"."}
                 ), "set root backup directory")
                ("threads,T",
                 po::value<size_t>()->default_value(8),
                 "set worker thread pool size");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            std::exit(EXIT_SUCCESS);
        }

        po::notify(vm);

        // canonical returns the absolute path resolving symbolic link, dot and dot-dot
        // it also checks the existence
        vm.at("backup-root").value() = fs::canonical(vm["backup-root"].as<fs::path>());
        auto backup_root = vm["backup-root"];
        if (backup_root.defaulted()) {
            std::cout << "--backup-root option set to default value: "
                      << backup_root.as<fs::path>() << std::endl;
        } else {
            auto dir = backup_root.as<fs::path>();
            if (!fs::is_directory(dir)) {
                std::cerr << dir << " is not a directory" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
        if (vm["threads"].defaulted()) {
            std::cout << "--threads option set to default value: "
                      << vm["threads"].as<size_t>() << std::endl;
        }

        return vm;
    }
    catch (std::exception &ex) {
        std::cout << "Error during options parsing: " << ex.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
// Parsing program options
    po::variables_map vm = parse_options(argc, argv);

    try {
        // Destructuring program options
        std::string server_addr = vm["address"].as<std::string>();
        std::string port =  vm["service"].as<std::string>();
        std::uint16_t num_threads = vm["threads"].as<size_t>();
        fs::path backup_root = vm["backup-root"].as<fs::path>();

        // Initialise the server.
        server s{server_addr, port, num_threads, backup_root};

        // Run the server until stopped.
        s.run();
    }
    catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return EXIT_SUCCESS;
}