//
// Created by leonardo on 01/12/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_REQUEST_HANDLER_H
#define REMOTE_BACKUP_M1_SERVER_REQUEST_HANDLER_H


#include <string>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include "message.h"
#include "dir.h"
#include "tlv_view.h"
#include "user.h"
#include <condition_variable>
#include <unordered_set>
#include "message_queue.h"
#include "logger.h"

typedef std::pair<
        std::unordered_map<std::string,
                std::shared_ptr<boost::filesystem::ofstream>>::iterator,
        bool
> get_stream_result;

class request_handler : private boost::noncopyable {
    boost::filesystem::path backup_root_;
    boost::filesystem::path credentials_path_;
    std::unordered_map<std::string, std::shared_ptr<boost::filesystem::ofstream>> opened_files_;
    std::mutex m_;

    static bool verify_password(
            boost::filesystem::path const &credentials_path,
            std::string const &username,
            std::string const &password
    );

    void handle_auth(communication::tlv_view &msg_view,
                     communication::message_queue &replies,
                     user &user);

    void handle_sync(communication::message_queue &replies,
                     user &user);

    void handle_create(communication::tlv_view &msg_view,
                       communication::message_queue &replies,
                       user &user);

    void handle_update(communication::tlv_view &msg_view,
                       communication::message_queue &replies,
                       user &user);

    void handle_erase(communication::tlv_view &msg_view,
                      communication::message_queue &replies,
                      user &user);

public:
    // Handle a request and produce a reply.
    explicit request_handler(
            boost::filesystem::path backup_root,
            boost::filesystem::path credentials_path
    );

    void handle_request(const communication::message &request,
                        communication::message_queue &replies,
                        user &usr);


    get_stream_result get_stream(std::string const &key,boost::filesystem::path const &path);
    void erase_stream(std::string const &key);
};

#endif //REMOTE_BACKUP_M1_SERVER_REQUEST_HANDLER_H
