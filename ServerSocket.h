//
// Created by leonardo on 12/10/20.
//

#ifndef SOCKET_SERVERSOCKET_H
#define SOCKET_SERVERSOCKET_H

#include <netinet/in.h>
#include "Socket.h"


class ServerSocket : private Socket {
public:
    ServerSocket(int port) {
        struct sockaddr_in sockaddrIn;
        sockaddrIn.sin_port = htons(port);
        sockaddrIn.sin_family = AF_INET;
        sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
        if (::bind(sockfd, reinterpret_cast<struct sockaddr *>(&sockaddrIn), sizeof(sockaddrIn)) != 0) {
            throw std::runtime_error("Cannot bind port");
        }
        if (::listen(sockfd, 8) != 0) {
            throw std::runtime_error("Cannot bind port");
        }
    }

    Socket accept(struct sockaddr_in *addr, unsigned int *len) {
        int fd = ::accept(sockfd, reinterpret_cast<struct sockaddr*>(addr), len);
        if (fd < 0) throw std::runtime_error("Cannot accept socket");
        return Socket(fd);
    }
};


#endif //SOCKET_SERVERSOCKET_H
