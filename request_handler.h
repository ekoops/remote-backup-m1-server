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


class request_handler : private boost::noncopyable {
    boost::filesystem::path backup_root_;
    std::unordered_map<std::string, std::shared_ptr<boost::filesystem::ofstream>> ofs_stream_;
    std::mutex m_;

    void handle_auth(communication::tlv_view &msg_view,
                     communication::message &reply,
                     user &user);

    void handle_sync(communication::message &reply,
                     user &user);

    void handle_create(communication::tlv_view &msg_view,
                       communication::message &reply,
                       user &user);

    void handle_update(communication::tlv_view &msg_view,
                       communication::message &reply,
                       user &user);

    void handle_delete(communication::tlv_view &msg_view,
                       communication::message &reply,
                       user &user);

public:
    explicit request_handler(boost::filesystem::path backup_root_);  /// Handle a request and produce a reply.
    void handle_request(const communication::message &request,
                        communication::message &reply,
                        user &usr);
};


#endif //REMOTE_BACKUP_M1_SERVER_REQUEST_HANDLER_H
