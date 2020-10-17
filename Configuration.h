//
// Created by leonardo on 17/10/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_CONFIGURATION_H
#define REMOTE_BACKUP_M1_SERVER_CONFIGURATION_H

#include <map>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

class Configuration {
    std::map<std::string, std::string> config;
public:
    Configuration(boost::filesystem::path const& config_path) {
        if (!boost::filesystem::exists(config_path)) {
            throw std::runtime_error{"Configuration not found"};
        }

        std::ifstream ifs{config_path};
        std::string line;
        boost::regex regex{"([a-z]{1,}):([a-z\\d]{1,})"};
        while (std::getline(ifs, line)) {
            auto pair = match_and_parse(regex, line);
            bool success = pair.first;
            std::vector<std::string> results = pair.second;
            if (!success || results.size() != 2) {
                throw std::runtime_error{"Failed to parse configuration file"};
            }
                // results[0]: key
                // results[1]: value
            else this->config[results[0]] = results[1];
        }
        ifs.close();
    }

    std::string get(std::string const& key) const {
        auto result = this->config.find(key);
        if (result != this->config.end()) {
            return result->second;
        }
        else {
            std::ostringstream oss;
            oss << "Failed to load " << key << " conf";
            throw std::runtime_error {oss.str()};
        };
    }
};


#endif //REMOTE_BACKUP_M1_SERVER_CONFIGURATION_H
