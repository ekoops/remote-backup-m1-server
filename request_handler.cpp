//
// Created by leonardo on 01/12/20.
//
#include "request_handler.h"
#include "tools.h"
#include "resource.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

request_handler::request_handler(fs::path backup_root_) : backup_root_{std::move(backup_root_)} {}

void close_response(communication::message &reply, communication::TLV_TYPE tlv_type) {
    reply.add_TLV(tlv_type);
    reply.add_TLV(communication::TLV_TYPE::END);
}

void request_handler::handle_auth(communication::tlv_view &msg_view,
                                  communication::message &reply,
                                  user& user) {
    std::string username;
    std::string password;
    if (msg_view.get_tlv_type() != communication::TLV_TYPE::USRN)
        return close_response(reply, communication::TLV_TYPE::ERROR);

    username = std::string{msg_view.cbegin(), msg_view.cend()};

    if (!msg_view.next_tlv() || msg_view.get_tlv_type() != communication::TLV_TYPE::PSWD)
        return close_response(reply, communication::TLV_TYPE::ERROR);

    password = std::string{msg_view.cbegin(), msg_view.cend()};

    if (tools::verify_password(username, password)) {
        std::string dir_name = tools::hash(username);
        user.dir(this->backup_root_.generic_path() / dir_name);
        user.auth(true);
        return close_response(reply, communication::TLV_TYPE::OK);
    }
    else return close_response(reply, communication::TLV_TYPE::ERROR);

}

void request_handler::
handle_sync(communication::message &reply, user& user) {
    auto user_dir = user.dir();
    fs::path const& user_dir_path = user_dir->path();
    size_t user_dir_path_length = user_dir_path.size();
    try {
        for (auto &de : fs::recursive_directory_iterator(user_dir_path)) {
            fs::path const &absolute_path = de.path();
            if (fs::is_regular_file(absolute_path)) {
                fs::path relative_path{absolute_path.generic_path().string().substr(user_dir_path_length)};
                std::string digest = tools::hash(absolute_path, relative_path);
                if (!user_dir->insert_or_assign(relative_path, directory::resource{
                        true,
                        digest
                }))
                    return close_response(reply, communication::TLV_TYPE::ERROR);

                std::string sign = tools::create_sign(relative_path, digest);
                reply.add_TLV(communication::TLV_TYPE::ITEM, sign.size(), sign.c_str());
            }
        }
        user.synced(true);
        close_response(reply, communication::TLV_TYPE::OK);
    }
    catch (fs::filesystem_error& ex) {
        std::cout << "Filesystem error from " << ex.what() << std::endl;
        reply = communication::message {communication::MESSAGE_TYPE::SYNC};
        close_response(reply, communication::TLV_TYPE::ERROR);
    }
}


void request_handler::handle_create(communication::tlv_view &msg_view,
                                    communication::message &reply,
                                    user& user) {
    if (msg_view.get_tlv_type() != communication::TLV_TYPE::ITEM)
        return close_response(reply, communication::TLV_TYPE::ERROR);
    std::string c_sign{msg_view.cbegin(), msg_view.cend()};
    auto splitted_c_sign = tools::split_sign(c_sign);
    fs::path &c_relative_path = splitted_c_sign.first;
    std::string c_digest = splitted_c_sign.second;
    auto user_dir = user.dir();

    reply.add_TLV(communication::TLV_TYPE::ITEM, c_sign.size(), c_sign.c_str());

    if (!msg_view.next_tlv() ||
        msg_view.get_tlv_type() != communication::TLV_TYPE::CONTENT)
        return close_response(reply, communication::TLV_TYPE::ERROR);

    auto rsrc = user_dir->rsrc(c_relative_path);
    if (rsrc && rsrc.value().synced()) return close_response(reply, communication::TLV_TYPE::ERROR);

    fs::path absolute_path{user_dir->path() / c_relative_path};
    try {
        create_directories(absolute_path.parent_path());
        std::shared_ptr<fs::ofstream> ofs_ptr;
        bool is_first;
        {
            std::unique_lock ul{this->m_};
            auto result = this->ofs_stream_.emplace(
                    c_sign,
                    std::make_shared<fs::ofstream>(absolute_path, std::ios_base::binary | std::ios_base::app)
            );
            ofs_ptr = result.first->second;
            is_first = result.second;       // inserted in the map
        }
        // writing file data
        std::copy(msg_view.cbegin(), msg_view.cend(), std::ostreambuf_iterator<char>(*ofs_ptr));

        bool is_last = msg_view.next_tlv() && msg_view.get_tlv_type() == communication::TLV_TYPE::END;

        if (is_first) {
            user_dir->insert_or_assign(c_relative_path, directory::resource{
                    is_last,    // if is the last chunked, then the file is synced on server
                    c_digest    // digest
            });
        }
        if (is_last) {
            ofs_ptr->close();
            this->ofs_stream_.erase(c_sign);
            // if is both first and last, then synced is already set to true and the following
            // operation isn't needed.
            if (!is_first) user_dir->insert_or_assign(c_relative_path, rsrc->synced(true));

            try {
                // Comparing server file digest with the sent digest
                std::string s_digest = tools::hash(absolute_path, c_relative_path);
                if (s_digest != c_digest) {
                    remove(absolute_path);  // if digests doesn't match, remove created file
                    return close_response(reply, communication::TLV_TYPE::ERROR);
                }
            }
            catch (fs::filesystem_error &ex) {
                std::cout << "Failed to access to file:\n\t" << absolute_path << std::endl;
                // TODO
                std::exit(-1);
            }
        }
        return close_response(reply, communication::TLV_TYPE::OK);
    }
    catch (fs::filesystem_error &ex) {
        //TODO
        std::cerr << "Filesystem error from " << ex.what() << std::endl;
        return close_response(reply, communication::TLV_TYPE::ERROR);
    }
}

void request_handler::handle_update(communication::tlv_view &msg_view,
                                    communication::message &reply,
                                    user& user) {
    if (msg_view.get_tlv_type() != communication::TLV_TYPE::ITEM)
        return close_response(reply, communication::TLV_TYPE::ERROR);
    std::string c_sign{msg_view.cbegin(), msg_view.cend()};
    auto splitted_c_sign = tools::split_sign(c_sign);
    fs::path &c_relative_path = splitted_c_sign.first;
    std::string c_digest = splitted_c_sign.second;
    auto user_dir = user.dir();

    reply.add_TLV(communication::TLV_TYPE::ITEM, c_sign.size(), c_sign.c_str());

    if (!msg_view.next_tlv() ||
        msg_view.get_tlv_type() != communication::TLV_TYPE::CONTENT)
        return close_response(reply, communication::TLV_TYPE::ERROR);

    auto rsrc = user_dir->rsrc(c_relative_path);
    if (!rsrc || rsrc.value().digest() == c_digest) return close_response(reply, communication::TLV_TYPE::ERROR);

    fs::path absolute_path{user_dir->path() / c_relative_path};
    fs::path temp_path{absolute_path};
    temp_path += ".temp";

    try {
        std::shared_ptr<fs::ofstream> ofs_ptr;
        bool is_first;
        {
            std::unique_lock ul{this->m_};
            auto result = this->ofs_stream_.emplace(
                    c_sign,
                    std::make_shared<fs::ofstream>(temp_path, std::ios_base::binary | std::ios_base::app)
            );
            ofs_ptr = result.first->second;
            is_first = result.second;
        }
        std::copy(msg_view.cbegin(), msg_view.cend(), std::ostreambuf_iterator<char>(*ofs_ptr));

        bool is_last = msg_view.next_tlv() && msg_view.get_tlv_type() == communication::TLV_TYPE::END;

        if (is_first) {
            user_dir->insert_or_assign(c_relative_path, directory::resource{
                    is_last,
                    c_digest
            });
        }
        if (is_last) {
            ofs_ptr->close();
            this->ofs_stream_.erase(c_sign);
            if (!remove(absolute_path)) std::exit(-1);
            rename(temp_path, absolute_path);
            if (!is_first) user_dir->insert_or_assign(c_relative_path, rsrc->synced(true));
            std::string digest = tools::hash(absolute_path, c_relative_path);
            if (digest != c_digest) {
                remove(absolute_path);
                // TODO eliminare il file dal server
                return close_response(reply, communication::TLV_TYPE::ERROR);
            }
        }
        return close_response(reply, communication::TLV_TYPE::OK);
    }
    catch (fs::filesystem_error &ex) {
        //TODO
        return close_response(reply, communication::TLV_TYPE::ERROR);
    }
}

void request_handler::handle_erase(communication::tlv_view &msg_view,
                                   communication::message &reply,
                                   user& user) {
    if (msg_view.get_tlv_type() != communication::TLV_TYPE::ITEM)
        return close_response(reply, communication::TLV_TYPE::ERROR);
    std::string c_sign{msg_view.cbegin(), msg_view.cend()};
    auto splitted_c_sign = tools::split_sign(c_sign);
    fs::path &c_relative_path = splitted_c_sign.first;
    std::string c_digest = splitted_c_sign.second;
    auto user_dir = user.dir();

    reply.add_TLV(communication::TLV_TYPE::ITEM, c_sign.size(), c_sign.c_str());

    auto rsrc = user_dir->rsrc(c_relative_path);
    if (!rsrc || rsrc.value().digest() != c_digest) return close_response(reply, communication::TLV_TYPE::ERROR);

    fs::path absolute_path{user_dir->path() / c_relative_path};
    fs::path temp{absolute_path};
    try {
        do {
            remove(temp);
            temp = temp.parent_path();
        } while (temp != user_dir->path() && is_empty(temp));
        return close_response(
                reply,
                user_dir->erase(c_relative_path)
                ? communication::TLV_TYPE::OK
                : communication::TLV_TYPE::ERROR
        );
    }
    catch (fs::filesystem_error &ex) {
        //TODO
        return close_response(reply, communication::TLV_TYPE::ERROR);
    }
}


void request_handler::handle_request(const communication::message &request,
                                     communication::message &reply,
                                     user& user) {
    auto c_msg_type = request.get_msg_type();
    reply = communication::message{c_msg_type};
    communication::tlv_view msg_view{request};
    if (!msg_view.next_tlv()) {
        return close_response(reply, communication::TLV_TYPE::ERROR);
    }
    if (!user.auth()) {
        if (c_msg_type == communication::MESSAGE_TYPE::AUTH) {
            return handle_auth(msg_view, reply, user);
        } else return close_response(reply, communication::TLV_TYPE::ERROR);
    } else {
        if (!user.synced()) {
            if (c_msg_type == communication::MESSAGE_TYPE::SYNC) {
                return handle_sync(reply, user);
            } else return close_response(reply, communication::TLV_TYPE::ERROR);
        } else {
            if (c_msg_type == communication::MESSAGE_TYPE::CREATE) {
                return handle_create(msg_view, reply, user);
            } else if (c_msg_type == communication::MESSAGE_TYPE::UPDATE) {
                return handle_update(msg_view, reply, user);
            } else if (c_msg_type == communication::MESSAGE_TYPE::ERASE) {
                return handle_erase(msg_view, reply, user);
            } else return close_response(reply, communication::TLV_TYPE::ERROR);
        }
    }
}