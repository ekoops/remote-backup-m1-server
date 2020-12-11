//
// Created by leonardo on 01/12/20.
//

#include "tools.h"
#include "tools.h"
#include <sstream>
#include <boost/algorithm/hex.hpp>
#include <openssl/sha.h>
#include <iostream>

using boost::uuids::detail::md5;
namespace fs = boost::filesystem;


std::string tools::create_sign(fs::path const &path,
                               std::string const &digest) {
    std::ostringstream oss;
    oss << path.generic_path().string() << '\x00' << digest;
    return oss.str();
}

std::pair<fs::path, std::string> tools::split_sign(std::string const& sign) {
    std::istringstream oss{sign};
    std::string temp;
    boost::regex regex {"(.*)\\x00(.*)"};
    auto pair = tools::match_and_parse(regex, sign);
    if (pair.first) return {pair.second[0], pair.second[1]};
    else throw std::runtime_error{"errore"};
//    std::vector<std::string> results(2);
//    while (std::getline(oss, temp, '\x00')) results.push_back(temp);
//    return results;
}

std::pair<bool, std::vector<std::string>> tools::match_and_parse(boost::regex const &regex, std::string const &line) {
    boost::smatch match_results{};
    std::vector<std::string> results;
    if (boost::regex_match(line, match_results, regex)) {
        for (int i = 1; i < match_results.size(); i++) {
            results.emplace_back(match_results[i]);
        }
        return {true, results};
    } else return {false, results};
}

std::string tools::hash(std::string const& s) {
    md5 hash;
    md5::digest_type digest;
    hash.process_bytes(s.data(), s.size());
    hash.get_digest(digest);
    return hash_to_string(digest);
}


std::string tools::hash(fs::path const &absolute_path, fs::path const& relative_path) {
    fs::ifstream ifs;
    ifs.open(absolute_path, std::ios_base::binary);
    ifs.unsetf(std::ios::skipws);           // Stop eating new lines in binary mode!!!
    std::streampos length;
    ifs.seekg(0, std::ios::end);
    length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    md5 hash;
    md5::digest_type digest;

    std::vector<char> file_buffer(length);
    file_buffer.reserve(length);
    file_buffer.insert(file_buffer.begin(),
                       std::istream_iterator<char>(ifs),
                       std::istream_iterator<char>());

    std::string relative_path_str {relative_path.generic_path().string()};
    hash.process_bytes(relative_path_str.c_str(), relative_path_str.size());
    hash.process_bytes(&*file_buffer.cbegin(), length);
    hash.get_digest(digest);

//    std::cout << "HASH1 " << hash_to_string(digest) << std::endl;
    return hash_to_string(digest);
}


std::string tools::hash_to_string(md5::digest_type const& digest) {
    const auto intDigest = reinterpret_cast<const int*>(&digest);
    std::string result;

    boost::algorithm::hex(intDigest, intDigest + (sizeof(md5::digest_type)/sizeof(int)), std::back_inserter(result));
    return result;
}

bool tools::verify_password(
        fs::path const& credentials_path,
        std::string const& username,
        std::string const& password) {
    char hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char *>(password.c_str()),
           password.size() - 1,
           reinterpret_cast<unsigned char *>(hash));
    std::string hash_str;
    boost::algorithm::hex(hash, hash + SHA512_DIGEST_LENGTH, std::back_inserter(hash_str));
    std::cout << hash_str << std::endl;

//     TODO generalizzare il path
    fs::ifstream ifs {credentials_path};
    if (!ifs) {
        std::cerr << "Failed to access to credentials" << std::endl;
        return false;
    }
    std::string line;
    while(getline(ifs, line)) {
        if (line.find(username) != std::string::npos) {
            if (line.find('\t') != std::string::npos) {
                std::string hashed_pswd = line.substr(line.find('\t')+1);
                return hashed_pswd == hash_str;
            } else return false;
        }
    }
    return false;
}