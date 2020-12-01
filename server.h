//
// Created by leonardo on 01/12/20.
//

#ifndef REMOTE_BACKUP_M1_SERVER_SERVER_H
#define REMOTE_BACKUP_M1_SERVER_SERVER_H


#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.h"

class server : private boost::noncopyable {
    std::size_t thread_pool_size_;
    boost::asio::io_context io_;
    boost::asio::ssl::context ctx_;
    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;
    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::shared_ptr<connection> new_connection_ptr;
    std::shared_ptr<request_handler> req_handler_ptr;
public:
    explicit server(std::string const &address,
                    std::string const& port,
                    std::uint16_t thread_pool_size,
                    boost::filesystem::path const &backup_root);

    void run();

private:
    std::string get_password() const;
    void start_accept();
    void handle_accept(const boost::system::error_code &e);
    void handle_stop();
};


#endif //REMOTE_BACKUP_M1_SERVER_SERVER_H
