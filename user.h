//
// Created by leonardo on 01/12/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_USER_H
#define REMOTE_BACKUP_M1_SERVER_USER_H


#include "dir.h"

class user {
    std::string username_;
    bool is_auth_;
    bool is_synced_;
    std::shared_ptr<directory::dir> dir_ptr_;
public:
    [[nodiscard]] std::string const& username() const {
        return this->username_;
    }
    void username(std::string const& username) {
        this->username_ = username;
    }
    [[nodiscard]] bool auth() const {
        return this->is_auth_;
    }
    void auth(bool is_auth) {
        this->is_auth_ = is_auth;
    }
    [[nodiscard]] bool synced() const {
        return this->is_synced_;
    }
    void synced(bool is_synced) {
        this->is_synced_ = is_synced;
    }
    std::shared_ptr<directory::dir> dir() {
        return this->dir_ptr_;
    }
    void dir(boost::filesystem::path const& absolute_path) {
        this->dir_ptr_ = directory::dir::get_instance(absolute_path);
    }
};


#endif //REMOTE_BACKUP_M1_SERVER_USER_H
