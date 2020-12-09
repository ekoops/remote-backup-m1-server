//
// Created by leonardo on 01/12/20.
//

#include "connection.h"
#include <vector>
#include <boost/bind/bind.hpp>
#include "message.h"
#include "message_queue.h"

#define BUFFER_SIZE 4096


connection::connection(boost::asio::io_context &io,
                       boost::asio::ssl::context &ctx,
                       std::shared_ptr<request_handler> req_handler)
        : strand_(boost::asio::make_strand(io)),
          socket_{strand_, ctx},
          req_handler_{std::move(req_handler)},
          request_buffer_{std::make_shared<std::vector<uint8_t>>(BUFFER_SIZE)},
          replies_{} {
    // TODO sistemare il 4096
}

ssl_socket::lowest_layer_type &connection::socket() {
    return this->socket_.lowest_layer();
}

void connection::shutdown() {
    std::cout << "SHUTDOWN" << std::endl;
    boost::system::error_code ignored_ec;
    this->socket_.shutdown(ignored_ec);
    this->socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

//void connection::write_response(boost::system::error_code const &e) {
//    if (!e) {
//        boost::asio::async_write(
//                this->socket_,
//                this->creply_.buffer(),
//                boost::bind(&connection::read_header,
//                            shared_from_this(),
//                            boost::asio::placeholders::error)
//        );
//    } else this->shutdown();
//}
void connection::write_response(boost::system::error_code const &e) {
    if (!e) {
        this->creply_ = this->replies_->front();
        this->replies_->pop();
        std::cout << this->creply_ << std::endl;
        boost::asio::async_write(
                this->socket_,
                this->creply_.buffer(),
                boost::bind(
                        this->replies_->empty() ? &connection::read_header : &connection::write_header,
                        shared_from_this(),
                        boost::asio::placeholders::error)
        );
    } else this->shutdown();
}

void connection::write_header(boost::system::error_code const &e) {
    if (!e) {
        this->creply_header_ = this->replies_->front().size();
        std::cout << "HEADER: " << this->creply_header_ << std::endl;
        boost::asio::async_write(
                this->socket_,
                boost::asio::buffer(&this->creply_header_, sizeof(this->creply_header_)),
                boost::bind(&connection::write_response,
                            shared_from_this(),
                            boost::asio::placeholders::error));
    } else this->shutdown();
}


void connection::handle_buffer(boost::system::error_code const &e) {
    if (!e) {
        try {
            std::cout << "HANDLE_RESPONSE" << std::endl;
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

            std::cout << "<<<<<<<<<<<<RESPONSE>>>>>>>>>>>>" << std::endl;

            this->creply_header_ = this->replies_->front().size();
            std::cout << "HEADER: " << this->creply_header_ << std::endl;
            boost::asio::async_write(
                    this->socket_,
                    boost::asio::buffer(&this->creply_header_, sizeof(this->creply_header_)),
                    boost::bind(&connection::write_response,
                                shared_from_this(),
                                boost::asio::placeholders::error));


        } catch (std::exception const &e) {
            std::cout << e.what() << std::endl;
            this->shutdown();
        }
    } else {
        std::cout << e << std::endl;
        this->shutdown();
    }
}

void connection::read_buffer(boost::system::error_code const &e) {
    if (!e) {
        boost::asio::async_read(
                this->socket_,
                boost::asio::buffer(*this->request_buffer_, this->request_header_),
                boost::bind(
                        &connection::handle_buffer,
                        shared_from_this(),
                        boost::asio::placeholders::error
                )
        );
    } else this->shutdown();
}

void connection::read_header(boost::system::error_code const &e) {
    if (!e) {
        boost::asio::async_read(
                this->socket_,
                boost::asio::buffer(&this->request_header_, sizeof(this->request_header_)),
                boost::bind(
                        &connection::read_buffer,
                        shared_from_this(),
                        boost::asio::placeholders::error
                )
        );
    } else this->shutdown();
}

void connection::start() {
    this->socket_.async_handshake(boost::asio::ssl::stream_base::server,
                                  boost::bind(&connection::read_header,
                                              shared_from_this(),
                                              boost::asio::placeholders::error));
}