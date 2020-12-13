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
          logger_ptr_{std::move(logger_ptr)},
          req_handler_ptr_{std::move(req_handler)},
          buffer_(communication::message_queue::CHUNK_SIZE) {
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

void connection::log_read() {
    this->logger_ptr_->log(this->user_,
                           communication::MESSAGE_TYPE::NONE,
                           RESULT_TYPE::NONE,
                           RESULT_TYPE::ERROR);
}

void connection::log_write(boost::system::error_code const &e) {
    auto message_result = !this->replies_.verify_error() ? RESULT_TYPE::OK : RESULT_TYPE::ERROR;
    auto connection_result = !e ? RESULT_TYPE::OK : RESULT_TYPE::ERROR;
    this->logger_ptr_->log(this->user_, this->replies_.msg_type(), message_result, connection_result);
}

void connection::handle_completion(boost::system::error_code const &e) {
    this->log_write(e);
    if (e) this->shutdown();
    else this->read_request(e);
}

void connection::write_response(boost::system::error_code const &e) {
    if (e) {
        this->log_write(e);
        return this->shutdown();
    }
    this->header_ = this->replies_.front().size();
    std::cout << "<<<<<<<<<<<<RESPONSE>>>>>>>>>>>>" << std::endl;
    std::cout << "HEADER: " << this->header_ << std::endl;
    boost::asio::async_write(
            this->socket_,
            boost::asio::buffer(&this->header_, sizeof(this->header_)),
            [self = shared_from_this()](boost::system::error_code const &e, size_t /**/) {
                if (e) {
                    self->log_write(e);
                    return self->shutdown();
                }
                self->msg_ = self->replies_.front();
                self->replies_.pop();
                std::cout << self->msg_ << std::endl;
                boost::asio::async_write(
                        self->socket_,
                        self->msg_.buffer(),
                        boost::bind(
                                self->replies_.empty()
                                ? &connection::handle_completion
                                : &connection::write_response,
                                self->shared_from_this(),
                                boost::asio::placeholders::error
                        )
                );
            }
    );
}

void connection::handle_request(boost::system::error_code const &e) {
    if (e) {
        this->log_read();
        return this->shutdown();
    }
    try {
        this->msg_ = communication::message{
                std::make_shared<std::vector<uint8_t>>(
                        this->buffer_.begin(),
                        std::next(this->buffer_.begin(), this->header_)
                )
        };
    } catch (std::exception const &e) {
        std::cout << e.what() << std::endl;
        return this->shutdown();
    }

    std::cout << "<<<<<<<<<<<<REQUEST>>>>>>>>>>>>" << std::endl;
    std::cout << this->msg_;

    this->req_handler_ptr_->handle_request(
            this->msg_,
            this->replies_,
            this->user_
    );
    this->write_response(boost::system::error_code{});
}

void connection::read_request(boost::system::error_code const &e) {
    if (e) {
        this->log_read();
        return this->shutdown();
    }
    boost::asio::async_read(
            this->socket_,
            boost::asio::buffer(&this->header_, sizeof(this->header_)),
            [self = shared_from_this()](boost::system::error_code const &e, size_t /**/) {
                if (e) {
                    self->log_read();
                    return self->shutdown();
                }
                boost::asio::async_read(
                        self->socket_,
                        boost::asio::buffer(self->buffer_, self->header_),
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