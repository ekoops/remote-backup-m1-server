//
// Created by leonardo on 01/12/20.
//

#include "server.h"
#include <boost/thread/thread.hpp>
#include <boost/bind/bind.hpp>
#include <vector>

server::server(std::string const &address,
               std::string const &port,
               std::uint16_t thread_pool_size,
               boost::filesystem::path const &backup_root)
        : thread_pool_size_{thread_pool_size},
          signals_{io_},
          acceptor_{io_},
          ctx_{boost::asio::ssl::context::sslv23}, // set generic ssl/tls version
          new_connection_ptr{},
          req_handler_ptr{std::make_shared<request_handler>(backup_root)} {
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    this->signals_.add(SIGINT);
    this->signals_.add(SIGTERM);
#if defined(SIGQUIT)
    this->signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    this->signals_.async_wait(boost::bind(&server::handle_stop, this));

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_);
    boost::asio::ip::tcp::endpoint endpoint =
            *resolver.resolve(address, port).begin();
    this->acceptor_.open(endpoint.protocol());
    this->acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    this->acceptor_.bind(endpoint);
    this->acceptor_.listen();

    // Setting up ssl context parameters
    this->ctx_.set_options(
            boost::asio::ssl::context::default_workarounds  // ssl bug workarounds
            | boost::asio::ssl::context::no_sslv2             // disable support for sslv2
            | boost::asio::ssl::context::single_dh_use);       // Always create a new key when using tmp_dh parameters
    this->ctx_.set_password_callback(boost::bind(&server::get_password, this));
    this->ctx_.use_certificate_chain_file("../certs/server-cert.pem");  // loading server pem certificate
    this->ctx_.use_private_key_file("../certs/server-key.pem", boost::asio::ssl::context::pem);
    this->ctx_.use_tmp_dh_file("../certs/dh2048.pem");

    start_accept();
}


std::string server::get_password() const {
    return "password";
}

void server::run() {
    // Create a pool of threads to run all of the io_contexts.
    std::vector<std::thread> threads;
    threads.reserve(this->thread_pool_size_);
    for (int i = 0; i < this->thread_pool_size_; i++) {
        threads.emplace_back(boost::bind(&boost::asio::io_context::run, &io_));
    }

    // Wait for all threads in the pool to exit.
    for (auto &t: threads) t.join();
}

void server::start_accept() {
    new_connection_ptr.reset(new connection(
            this->io_,
            this->ctx_,
            this->req_handler_ptr)
    );
    acceptor_.async_accept(new_connection_ptr->raw_socket(),
                           boost::bind(&server::handle_accept, this,
                                       boost::asio::placeholders::error));

}

void server::handle_accept(const boost::system::error_code &e) {
    std::cout << "Accepted connection" << std::endl;
    if (!e) {
        this->new_connection_ptr->start();
    }

    start_accept();
}

void server::handle_stop() {
    this->io_.stop();
}
