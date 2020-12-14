#ifndef REMOTE_BACKUP_M1_SERVER_USER_H
#define REMOTE_BACKUP_M1_SERVER_USER_H

#include "../directory/dir.h"

/*
 * This class is used to
 */
class user {
    std::string id_;
    std::string username_;
    std::string ip_;
    bool is_auth_;
    bool is_synced_;
    std::shared_ptr<directory::dir> dir_ptr_;
public:
    [[nodiscard]] std::string const &id() const { return this->id_; }

    user &id(std::string const &id) {
        this->id_ = id;
        return *this;
    }

    [[nodiscard]] std::string const &ip() const { return this->ip_; }

    user &ip(std::string const &ip) {
        this->ip_ = ip;
        return *this;
    }

    [[nodiscard]] std::string const &username() const {
        return this->username_;
    }

    user &username(std::string const &username) {
        this->username_ = username;
        return *this;
    }

    [[nodiscard]] bool auth() const {
        return this->is_auth_;
    }

    user &auth(bool is_auth) {
        this->is_auth_ = is_auth;
        return *this;
    }

    [[nodiscard]] bool synced() const {
        return this->is_synced_;
    }

    user &synced(bool is_synced) {
        this->is_synced_ = is_synced;
        return *this;
    }

    std::shared_ptr<directory::dir> dir() {
        return this->dir_ptr_;
    }

    user &dir(boost::filesystem::path const &absolute_path) {
        this->dir_ptr_ = directory::dir::get_instance(absolute_path);
        return *this;
    }
    bool operator==(user const &other) const {
        return this->id_ == other.id_;
    }
};

namespace std {
    template <>
    struct hash<user> {
        std::size_t operator()(user const& user) const {
            return std::hash<std::string>()(user.id());
        }
    };
}

#endif //REMOTE_BACKUP_M1_SERVER_USER_H
