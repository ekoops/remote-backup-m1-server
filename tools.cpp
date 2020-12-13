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

std::pair<fs::path, std::string> tools::split_sign(std::string const &sign) {
    std::istringstream oss{sign};
    std::string temp;
    boost::regex regex{"(.*)\\x00(.*)"};
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

std::string tools::MD5_hash(std::string const &s) {
    md5 hash;
    md5::digest_type digest;
    hash.process_bytes(s.data(), s.size());
    hash.get_digest(digest);
    return MD5_to_string(digest);
}


std::string tools::MD5_hash(fs::path const &absolute_path, fs::path const &relative_path) {
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

    std::string relative_path_str{relative_path.generic_path().string()};
    hash.process_bytes(relative_path_str.c_str(), relative_path_str.size());
    hash.process_bytes(&*file_buffer.cbegin(), length);
    hash.get_digest(digest);

//    std::cout << "HASH1 " << MD5_to_string(digest) << std::endl;
    return MD5_to_string(digest);
}


/**
* Allow to obtain a string representation from an MD5 digest
*
* @param digest the digest that has to be converted
* @return a string representation of an MD5 digest
*/
std::string tools::MD5_to_string(boost::uuids::detail::md5::digest_type const &digest) {
    const auto int_digest = reinterpret_cast<const int *>(&digest);
    std::string result;

    boost::algorithm::hex(int_digest, int_digest + (sizeof(md5::digest_type) / sizeof(int)),
                          std::back_inserter(result));
    return result;
}

std::string tools::SHA512_hash(std::string const &str) {
    char digest[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char *>(str.c_str()),
           str.size() - 1,
           reinterpret_cast<unsigned char *>(digest));
    std::string digest_str;
    boost::algorithm::hex(digest, digest + SHA512_DIGEST_LENGTH, std::back_inserter(digest_str));
    std::cout << digest_str << std::endl;
    return digest_str;
}