#include "request_handler.h"
#include "tools.h"
#include "resource.h"
#include <boost/filesystem.hpp>
#include <utility>
#include <boost/algorithm/hex.hpp>
#include "message_queue.h"

namespace fs = boost::filesystem;

request_handler::request_handler(
        fs::path backup_root,
        fs::path credentials_path
) : backup_root_{std::move(backup_root)},
    credentials_path_{std::move(credentials_path)} {}

void close_response(communication::message_queue &replies, communication::TLV_TYPE tlv_type) {
    replies.add_TLV(tlv_type);
    replies.add_TLV(communication::TLV_TYPE::END);
}

bool request_handler::verify_password(
        fs::path const &credentials_path,
        std::string const &username,
        std::string const &password) {
    std::string c_digest = std::move(tools::SHA512_hash(password));

    fs::ifstream ifs{credentials_path};
    if (!ifs) {
        std::cerr << "Failed to access to credentials" << std::endl;
        return false;
    }
    std::string line;
    while (getline(ifs, line)) {
        if (line.find(username) != std::string::npos) {
            if (line.find('\t') != std::string::npos) {
                std::string s_digest = line.substr(line.find('\t') + 1);
                return s_digest == c_digest;
            } else return false;
        }
    }
    return false;
}

void request_handler::handle_auth(
        communication::tlv_view &msg_view,
        communication::message_queue &replies,
        user &user
) {
    std::string username;
    std::string password;
    if (msg_view.tlv_type() != communication::TLV_TYPE::USRN)
        return close_response(replies, communication::TLV_TYPE::ERROR);

    username = std::string{msg_view.cbegin(), msg_view.cend()};

    if (!msg_view.next_tlv() || msg_view.tlv_type() != communication::TLV_TYPE::PSWD)
        return close_response(replies, communication::TLV_TYPE::ERROR);

    password = std::string{msg_view.cbegin(), msg_view.cend()};

    if (verify_password(this->credentials_path_, username, password)) {
        std::string user_id = tools::MD5_hash(username);
        user.id(user_id)
                .username(username)
                .dir(this->backup_root_.generic_path() / user_id)
                .auth(true);
        return close_response(replies, communication::TLV_TYPE::OK);
    } else return close_response(replies, communication::TLV_TYPE::ERROR);

}

void request_handler::handle_sync(
        communication::message_queue &replies,
        user &user
) {
    auto user_dir = user.dir();
    fs::path const &user_dir_path = user_dir->path();
    size_t user_dir_path_length = user_dir_path.size();

    try {
        for (auto &de : fs::recursive_directory_iterator(user_dir_path)) {
            fs::path const &absolute_path = de.path();
            if (fs::is_regular_file(absolute_path)) {
                fs::path relative_path{absolute_path.generic_path().string().substr(user_dir_path_length)};
                std::string digest = tools::MD5_hash(absolute_path, relative_path);
                if (!user_dir->insert_or_assign(relative_path, directory::resource{
                        true,
                        digest
                })) {
                    user_dir->clear();
                    return close_response(replies, communication::TLV_TYPE::ERROR);
                }

                std::string sign = tools::create_sign(relative_path, digest);
                replies.add_TLV(communication::TLV_TYPE::ITEM, sign.size(), sign.c_str());
            }
        }
        user.synced(true);
        close_response(replies, communication::TLV_TYPE::OK);
    }
    catch (fs::filesystem_error &ex) {
        std::cout << "Filesystem error:\n\t" << ex.what() << std::endl;
        user_dir->clear();
        replies = communication::message_queue{communication::MESSAGE_TYPE::SYNC};
        close_response(replies, communication::TLV_TYPE::ERROR);
    }
}


get_stream_result request_handler::get_stream(
        std::string const &key,
        fs::path const &path
) {
    std::unique_lock ul{this->m_};
    return this->opened_files_.emplace(
            key,
            std::make_shared<fs::ofstream>(path, std::ios_base::binary | std::ios_base::app)
    );
}

void request_handler::erase_stream(std::string const &key) {
    std::unique_lock ul{this->m_};
    this->opened_files_.erase(key);
}

void request_handler::handle_create(
        communication::tlv_view &msg_view,
        communication::message_queue &replies,
        user &user
) {
    if (msg_view.tlv_type() != communication::TLV_TYPE::ITEM)
        return close_response(replies, communication::TLV_TYPE::ERROR);
    std::string c_sign{msg_view.cbegin(), msg_view.cend()};
    auto splitted_c_sign = tools::split_sign(c_sign);
    fs::path &c_relative_path = splitted_c_sign.first;
    std::string c_digest = splitted_c_sign.second;
    auto user_dir = user.dir();

    replies.add_TLV(communication::TLV_TYPE::ITEM, c_sign.size(), c_sign.c_str());

    if (!msg_view.next_tlv() ||
        msg_view.tlv_type() != communication::TLV_TYPE::CONTENT)
        return close_response(replies, communication::TLV_TYPE::ERROR);

    auto rsrc = user_dir->rsrc(c_relative_path);
    // if the resource already exists on server and is completed...
    if (rsrc && rsrc.value().synced()) return close_response(replies, communication::TLV_TYPE::ERROR);

    fs::path absolute_path{user_dir->path() / c_relative_path};
    boost::system::error_code ec;
    // creating necessary file containing folders
    create_directories(absolute_path.parent_path(), ec);
    if (ec) return close_response(replies, communication::TLV_TYPE::ERROR);

    // Defining key to address stream on opened_file map
    auto stream_key = user.id() + c_sign;
    std::shared_ptr<fs::ofstream> ofs_ptr;
    bool is_last;
    try {
        auto result = this->get_stream(stream_key, absolute_path);
        // file stream ptr
        ofs_ptr = result.first->second;
        if (!ofs_ptr || !*ofs_ptr) return close_response(replies, communication::TLV_TYPE::ERROR);

        // writing file data
        std::copy(msg_view.cbegin(), msg_view.cend(), std::ostreambuf_iterator<char>(*ofs_ptr));
        ofs_ptr->flush();

        bool is_first = result.second;
        is_last = msg_view.verify_end();

        if (is_first) {
            user_dir->insert_or_assign(c_relative_path, directory::resource{
                    is_last, is_last ? c_digest : "TEMP"
            });
        } else if (is_last) {
            user_dir->insert_or_assign(c_relative_path, directory::resource{
                    true, c_digest
            });
        }
    } catch (fs::filesystem_error &ex) {
        std::cerr << "Filesystem error from " << ex.what() << std::endl;
        return close_response(replies, communication::TLV_TYPE::ERROR);
    }

    if (is_last) {
        ofs_ptr->close();
        erase_stream(stream_key);
        std::string s_digest;
        try {
            // Comparing server file digest with the sent digest
            s_digest = tools::MD5_hash(absolute_path, c_relative_path);
            if (s_digest != c_digest) {
                remove(absolute_path, ec);  // if digests doesn't match, remove created file
                if (ec) std::exit(EXIT_FAILURE);
                user_dir->erase(c_relative_path);
                return close_response(replies, communication::TLV_TYPE::ERROR);
            }
        }
        catch (fs::filesystem_error &ex) {
            std::cerr << "Failed to access to file:\n\t" << absolute_path << std::endl;
            remove(absolute_path, ec);
            if (ec) std::exit(EXIT_FAILURE);
            user_dir->erase(c_relative_path);
            return close_response(replies, communication::TLV_TYPE::ERROR);
        }
    }
    return close_response(replies, communication::TLV_TYPE::OK);
}

void request_handler::handle_update(communication::tlv_view &msg_view,
                                    communication::message_queue &replies,
                                    user &user) {
    if (msg_view.tlv_type() != communication::TLV_TYPE::ITEM) {
        return close_response(replies, communication::TLV_TYPE::ERROR);
    }
    std::string c_sign{msg_view.cbegin(), msg_view.cend()};
    auto splitted_c_sign = tools::split_sign(c_sign);
    fs::path &c_relative_path = splitted_c_sign.first;
    std::string c_digest = splitted_c_sign.second;
    auto user_dir = user.dir();

    replies.add_TLV(communication::TLV_TYPE::ITEM, c_sign.size(), c_sign.c_str());

    if (!msg_view.next_tlv() ||
        msg_view.tlv_type() != communication::TLV_TYPE::CONTENT) {
        return close_response(replies, communication::TLV_TYPE::ERROR);
    }

    auto rsrc = user_dir->rsrc(c_relative_path);
    if (!rsrc || rsrc.value().digest() == c_digest) {
        return close_response(replies, communication::TLV_TYPE::ERROR);
    }

    fs::path absolute_path{user_dir->path() / c_relative_path};
    fs::path temp_path{absolute_path};
    temp_path += ".temp";

    // Defining key to address stream on opened_file map
    auto stream_key = user.id() + c_sign;
    std::shared_ptr<fs::ofstream> ofs_ptr;
    bool is_first;
    bool is_last;
    try {
        get_stream_result result = this->get_stream(stream_key, temp_path);
        ofs_ptr = result.first->second;
        if (!ofs_ptr || !*ofs_ptr) return close_response(replies, communication::TLV_TYPE::ERROR);

        std::copy(msg_view.cbegin(), msg_view.cend(), std::ostreambuf_iterator<char>(*ofs_ptr));

        is_first = result.second;
        is_last = msg_view.verify_end();

        if (is_first) {
            user_dir->insert_or_assign(c_relative_path, directory::resource{
                    is_last, is_last ? c_digest : rsrc.value().digest()
            });
        } else if (is_last) {
            user_dir->insert_or_assign(c_relative_path, directory::resource{
                    true, c_digest
            });
        }
    } catch (fs::filesystem_error &ex) {
        std::cerr << "EXIT Exception: " << ex.what() << std::endl;
        return close_response(replies, communication::TLV_TYPE::ERROR);
    }

    if (is_last) {
        ofs_ptr->close();
        erase_stream(stream_key);
        boost::system::error_code ec;
        remove(absolute_path, ec);
        if (ec) std::exit(EXIT_FAILURE);
        rename(temp_path, absolute_path, ec);
        if (ec) std::exit(EXIT_FAILURE);
        std::string s_digest;
        try {
            // Comparing server file digest with the sent digest
            s_digest = tools::MD5_hash(absolute_path, c_relative_path);
            if (s_digest != c_digest) {
                remove(absolute_path, ec);  // if digests doesn't match, remove created file
                if (ec) std::exit(EXIT_FAILURE);
                user_dir->erase(c_relative_path);
                return close_response(replies, communication::TLV_TYPE::ERROR);
            }
        }
        catch (fs::filesystem_error &ex) {
            std::cerr << "Failed to access to file:\n\t" << absolute_path << std::endl;
            remove(absolute_path, ec);
            if (ec) std::exit(EXIT_FAILURE);
            user_dir->erase(c_relative_path);
            return close_response(replies, communication::TLV_TYPE::ERROR);
        }
    }
    return close_response(replies, communication::TLV_TYPE::OK);
}

void request_handler::handle_erase(communication::tlv_view &msg_view,
                                   communication::message_queue &replies,
                                   user &user) {
    if (msg_view.tlv_type() != communication::TLV_TYPE::ITEM)
        return close_response(replies, communication::TLV_TYPE::ERROR);
    std::string c_sign{msg_view.cbegin(), msg_view.cend()};
    auto splitted_c_sign = tools::split_sign(c_sign);
    fs::path &c_relative_path = splitted_c_sign.first;
    std::string c_digest = splitted_c_sign.second;
    auto user_dir = user.dir();

    replies.add_TLV(communication::TLV_TYPE::ITEM, c_sign.size(), c_sign.c_str());

    auto rsrc = user_dir->rsrc(c_relative_path);
    if (!rsrc || rsrc.value().digest() != c_digest) return close_response(replies, communication::TLV_TYPE::ERROR);

    fs::path absolute_path{user_dir->path() / c_relative_path};
    fs::path tmp{absolute_path};
    boost::system::error_code ec;
    remove(tmp, ec);
    close_response(
            replies,
            !ec && user_dir->erase(c_relative_path)
            ? communication::TLV_TYPE::OK
            : communication::TLV_TYPE::ERROR
    );
    if (ec) return;
    tmp = tmp.parent_path();
    while (!ec && tmp != user_dir->path() && is_empty(tmp, ec)) {
        remove(tmp, ec);
        tmp = tmp.parent_path();
    }
}

void request_handler::handle_request(
        const communication::message &request,
        communication::message_queue &replies,
        user &user
) {
    auto c_msg_type = request.msg_type();
    // assign the response message type to the reply.
    replies = communication::message_queue{c_msg_type};
    communication::tlv_view msg_view{request};
    if (!msg_view.next_tlv()) {
        return close_response(replies, communication::TLV_TYPE::ERROR);
    }
    if (!user.auth()) {
        if (c_msg_type == communication::MESSAGE_TYPE::AUTH) {
            return handle_auth(msg_view, replies, user);
        } else return close_response(replies, communication::TLV_TYPE::ERROR);
    } else {
        if (!user.synced()) {
            if (c_msg_type == communication::MESSAGE_TYPE::SYNC) {
                return handle_sync(replies, user);
            } else return close_response(replies, communication::TLV_TYPE::ERROR);
        } else {
            if (c_msg_type == communication::MESSAGE_TYPE::CREATE) {
                return handle_create(msg_view, replies, user);
            } else if (c_msg_type == communication::MESSAGE_TYPE::UPDATE) {
                return handle_update(msg_view, replies, user);
            } else if (c_msg_type == communication::MESSAGE_TYPE::ERASE) {
                return handle_erase(msg_view, replies, user);
            } else if (c_msg_type == communication::MESSAGE_TYPE::KEEP_ALIVE) {
                return close_response(replies, communication::TLV_TYPE::OK);
            } else return close_response(replies, communication::TLV_TYPE::ERROR);
        }
    }
}