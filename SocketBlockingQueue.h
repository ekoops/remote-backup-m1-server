//
// Created by leonardo on 17/10/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_SOCKETBLOCKINGQUEUE_H
#define REMOTE_BACKUP_M1_SERVER_SOCKETBLOCKINGQUEUE_H
#include <condition_variable>
#include <mutex>
#include <queue>
#include "Socket.h"

class SocketBlockingQueue {
    std::queue<Socket> socket_queue;
    std::condition_variable cv;
    std::mutex m;
public:
    void push(Socket&& socket) {
        std::unique_lock ul {m};
        this->socket_queue.push(std::move(socket));
        cv.notify_all();
    }

    Socket&& pop() {
        std::unique_lock ul {m};
        while (this->socket_queue.empty()) cv.wait(ul);
        Socket socket = std::move(this->socket_queue.front());
        this->socket_queue.pop();
        return std::move(socket);
    }
};


#endif //REMOTE_BACKUP_M1_SERVER_SOCKETBLOCKINGQUEUE_H
