//
// Created by leonardo on 07/12/20.
//

#ifndef REMOTE_BACKUP_M1_CLIENT_MESSAGE_VECTOR_H
#define REMOTE_BACKUP_M1_CLIENT_MESSAGE_VECTOR_H

#include "message.h"
#include <queue>


namespace communication {
    class message_queue {
        std::queue<communication::message> msgs_queue_;
        MESSAGE_TYPE msg_type_;
        bool error_;
    public:
        static size_t const CHUNK_SIZE;
        explicit message_queue(MESSAGE_TYPE msg_type = NONE);

        void add_TLV(TLV_TYPE tlv_type, size_t length = 0, char const *buffer = nullptr);

        void pop();
        message front();
        bool empty();
        [[nodiscard]] MESSAGE_TYPE msg_type() const;
        [[nodiscard]] bool verify_error() const;
    };
}


#endif //REMOTE_BACKUP_M1_CLIENT_MESSAGE_VECTOR_H
