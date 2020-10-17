//
// Created by leonardo on 12/10/20.
//

#ifndef SOCKET_SOCKET_H
#define SOCKET_SOCKET_H


#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

class Socket {
    int sockfd;
    Socket(int sockfd): sockfd {sockfd} {}
    Socket(Socket const&) = delete;
    Socket &operator=(Socket const&) = delete;
    friend class ServerSocket;
public:
    Socket() {
        this->sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->sockfd < 0) throw std::runtime_error("Cannot create socket");
    }
    Socket(Socket&& other): sockfd {other.sockfd} {
        other.sockfd = 0;
    }
    Socket &operator=(Socket&& other) {
        if (this->sockfd != 0) close(this->sockfd);
        this->sockfd = other.sockfd;
        other.sockfd = 0;
        return *this;
    }
    ~Socket() {
        if (this->sockfd != 0) close(this->sockfd);
    }

    ssize_t read(char *buffer, size_t len, int options) {
        ssize_t res = recv(this->sockfd, buffer, len, options);
        if (res < 0) throw std::runtime_error("Cannot receive from socket");
        return res;
    }
    ssize_t write(char const* buffer, size_t len, int options) {
        ssize_t res = send(this->sockfd, buffer, len, options);
        if (res < 0) throw std::runtime_error("Cannot write to socket");
        return res;
    }
    void connect(struct sockaddr_in *addr, unsigned int len) {
        if (::connect(this->sockfd, reinterpret_cast<struct sockaddr *>(addr), len) != 0) {
            throw std::runtime_error("Cannot connect to remote socket");
        }
    }
};


#endif //SOCKET_SOCKET_H
