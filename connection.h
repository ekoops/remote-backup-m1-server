//
// Created by leonardo on 01/12/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_CONNECTION_H
#define REMOTE_BACKUP_M1_SERVER_CONNECTION_H


#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>
#include <boost/array.hpp>
#include "dir.h"
#include "message.h"
#include "request_handler.h"
#include "user.h"

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

/// Represents a single connection from a client.
class connection : public boost::enable_shared_from_this<connection>, private boost::noncopyable {

    /// Strand to ensure the connection's handlers are not called concurrently.
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    /// Socket for the connection.
    ssl_socket socket_;
    std::shared_ptr<request_handler> req_handler_;

    size_t request_header_;
    std::shared_ptr<std::vector<uint8_t>> request_buffer_;
    communication::message request_;

    size_t reply_header_;
    communication::message reply_;
    user user_;

    void shutdown();

    void write_response(boost::system::error_code const &e);

    void handle_buffer(boost::system::error_code const &e);

    void read_buffer(boost::system::error_code const &e);

    void read_header(boost::system::error_code const &e);


public:

    /// Construct a connection with the given io.
    explicit connection(boost::asio::io_context &io,
                        boost::asio::ssl::context &ctx,
                        std::shared_ptr<request_handler> req_handler);

    ssl_socket::lowest_layer_type &socket();

    void start();
};


#endif //REMOTE_BACKUP_M1_SERVER_CONNECTION_H
