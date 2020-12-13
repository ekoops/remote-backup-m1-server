#include "message_queue.h"


using namespace communication;
namespace fs = boost::filesystem;

size_t const message_queue::CHUNK_SIZE = 4096;

message_queue::message_queue(
        MESSAGE_TYPE msg_type
) : msg_type_{msg_type}, msgs_queue_ {std::queue<message>{}} {
    this->msgs_queue_.emplace(this->msg_type_);
}


void message_queue::add_TLV(TLV_TYPE tlv_type, size_t length, const char *buffer) {
    // there is at least an element so empty check is useless
    auto &last = this->msgs_queue_.back();
    if (last.size() + 1 + 4 + length <= CHUNK_SIZE) {
        last.add_TLV(tlv_type, length, buffer);
    } else {
        message msg{this->msg_type_};
        msg.add_TLV(tlv_type, length, buffer);
        this->msgs_queue_.push(msg);
    }
    if (tlv_type == communication::TLV_TYPE::ERROR) this->error_ = true;
}

void message_queue::pop() {
    this->msgs_queue_.pop();
}

message message_queue::front() {
    return this->msgs_queue_.front();
}

bool message_queue::empty() {
    return this->msgs_queue_.empty();
}

MESSAGE_TYPE message_queue::msg_type() const {
    return this->msg_type_;
}

bool message_queue::verify_error() const {
    return this->error_;
}