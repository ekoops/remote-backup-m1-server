#include "connection.h"
#include <vector>
#include <boost/bind/bind.hpp>

#include "message.h"


connection::connection(
        boost::asio::io_context &io,
                       boost::asio::ssl::context &ctx,
                       std::shared_ptr<logger> logger_ptr,
                       std::shared_ptr<request_handler> req_handler)
        : strand_(boost::asio::make_strand(io)),
          socket_{strand_, ctx},
          logger_ptr_ {std::move(logger_ptr)},
          req_handler_{std::move(req_handler)},
          request_buffer_{std::make_shared<std::vector<uint8_t>>(communication::message_queue::CHUNK_SIZE)} {
        sum = 0;
        count = 0;
}

ssl_socket &connection::socket() {
    return this->socket_;
}

void connection::shutdown() {
    this->logger_ptr_->log(this->user_, "Shutdown");
    boost::system::error_code ignored_ec;
    this->socket_.shutdown(ignored_ec);
    this->socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

void connection::write_response(boost::system::error_code const &e) {
    if (e) return this->shutdown();
    this->current_reply_header_ = this->replies_->front().size();
    std::cout << "<<<<<<<<<<<<RESPONSE>>>>>>>>>>>>" << std::endl;
    std::cout << "HEADER: " << this->current_reply_header_ << std::endl;
    boost::asio::async_write(
            this->socket_,
            boost::asio::buffer(&this->current_reply_header_, sizeof(this->current_reply_header_)),
            [self = shared_from_this()](boost::system::error_code const &e, size_t /**/) {
                if (e) return self->shutdown();
                self->current_reply_ = self->replies_->front();
                self->replies_->pop();
                std::cout << self->current_reply_ << std::endl;
                boost::asio::async_write(
                        self->socket_,
                        self->current_reply_.buffer(),
                        boost::bind(
                                self->replies_->empty()
                                ? &connection::read_request
                                : &connection::write_response,
                                self->shared_from_this(),
                                boost::asio::placeholders::error)
                );
            }
    );
}


void connection::handle_request(boost::system::error_code const &e) {
    if (e) return this->shutdown();
    try {
//        std::cout << "HANDLE_RESPONSE" << std::endl;
        this->request_ = communication::message{
                std::make_shared<std::vector<uint8_t>>(
                        this->request_buffer_->begin(),
                        std::next(this->request_buffer_->begin(), this->request_header_)
                )
        };
        std::cout << "<<<<<<<<<<<<REQUEST>>>>>>>>>>>>" << std::endl;
        std::cout << this->request_;

        this->req_handler_->handle_request(
                this->request_,
                this->replies_,
                this->user_
        );
        this->write_response(boost::system::error_code{});

    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
        this->shutdown();
    }
}

void connection::read_request(boost::system::error_code const &e) {
    if (e) return this->shutdown();
    boost::asio::async_read(
            this->socket_,
            boost::asio::buffer(&this->request_header_, sizeof(this->request_header_)),
            [self = shared_from_this()](boost::system::error_code const &e, size_t /**/) {
                if (e) return self->shutdown();
                boost::asio::async_read(
                        self->socket_,
                        boost::asio::buffer(*self->request_buffer_, self->request_header_),
                        boost::bind(
                                &connection::handle_request,
                                self->shared_from_this(),
                                boost::asio::placeholders::error
                        )
                );
            }
    );
}

void connection::start() {
    this->user_.ip(this->socket_.lowest_layer().remote_endpoint().address().to_string());
    this->logger_ptr_->log(this->user_, "Accepted connection");
    this->socket_.lowest_layer().set_option(boost::asio::socket_base::keep_alive(true));
    this->socket_.async_handshake(boost::asio::ssl::stream_base::server,
                                  boost::bind(&connection::read_request,
                                              shared_from_this(),
                                              boost::asio::placeholders::error));
}