//
// Created by leonardo on 17/10/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_REGEXUTILITY_H
#define REMOTE_BACKUP_M1_SERVER_REGEXUTILITY_H
#include <string>
#include <tuple>
#include <vector>
#include <boost/regex.hpp>

class RegexUtility {
public:
    static std::pair<bool, std::vector<std::string>> match_and_parse(boost::regex const& regex, std::string const& line) {
        boost::regex e {"([a-z]{1,}):([a-z\\d]{1,})"};
        boost::smatch match_results {};
        std::vector<std::string> results;
        if (boost::regex_match(line, match_results, e)) {
            for (int i=1; i<match_results.size(); i++) {
                results.emplace_back(match_results[i-1]);
            }
            return {true, results};
        }
        else return {false, results};
    }

};


#endif //REMOTE_BACKUP_M1_SERVER_REGEXUTILITY_H
