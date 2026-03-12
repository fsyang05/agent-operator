#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <utility>

#include "http_tcp_server.hpp"

namespace
{
    template<typename... Args>
    void log(const Args&... args)
    {
        // unary right fold over comma operator
        // a1 op (a2 op (a3 op a4))
        ((std::cout << args << ' '), ...) << std::endl;
    }

    void exit_with_error(const std::string& error_msg)
    {
        log("(ERROR):", error_msg, "errno:", strerror(errno));
        exit(1);
    }

    constexpr std::pair<std::string_view, http::Method> method_map[] = {
        { "GET",    http::Method::GET },
        { "POST",   http::Method::POST },
    };
}

namespace http
{

    RequestParams::RequestParams(const std::string& raw_request)
    {
        std::stringstream ss(raw_request);
        std::string method;
        ss >> method;
        ss >> this->path;
        for (const auto& [sm, m] : method_map) {
            if (method == sm) this->method = m;
        }

        auto pos = raw_request.find("\r\n\r\n");
        if (pos == std::string::npos) {
            this->body = "";
        } else {
            this->body = raw_request.substr(pos + 4);
        }
    }

    ResponseParams::ResponseParams(StatusCode status, const std::string& body)
    {
        std::string status_str;
        switch (status) {
            case StatusCode::OK         : status_str = "OK"; break;
            case StatusCode::NOT_FOUND  : status_str = "Not Found"; break;
        }

        response = "HTTP/1.1 " + std::to_string(static_cast<int>(status))
                    + " " + status_str + "\r\n"
                    + "Content-Length: " + std::to_string(body.size()) + "\r\n"
                    + "\r\n"
                    + body;
    }

    TCPServer::TCPServer(const int& port)
    {
        port_ = port;
        start_server();
    }

    TCPServer::~TCPServer()
    {
        close_server();
    }

    int TCPServer::start_server()
    {
        // create socket
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ == -1) {
            exit_with_error("socket not created");
            return -1;
        }

        // bind socket to a port
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family        = AF_INET;
        addr_.sin_port          = htons(port_);
        addr_.sin_addr.s_addr   = INADDR_ANY;

        if (bind(socket_fd_, (sockaddr*)&addr_, sizeof(addr_)) == -1) {
            exit_with_error("unable to bind for some reason");
            return -1;
        }

        return 0;
    }

    void TCPServer::register_route(const std::string& path, Method method, Handler handler)
    {
        // move semantics
        // turn handler from an lvalue into an rvalue. faster than copy
        //
        // basically tells the compiler that you can empty out handler
        // since it is effectively stored in the map now
        routes_[path][(size_t)method] = std::move(handler);
    }

    int TCPServer::start_listen()
    {
        if (listen(socket_fd_, 128) == -1) {
            exit_with_error("socket cannot start listening");
            return -1;
        }

        log("listening on addr",
            inet_ntoa(addr_.sin_addr),
            "on port",
            ntohs(addr_.sin_port));

        log("all registered routes:");
        for (auto& it : routes_) {
            log(it.first);
        }

        // we'd want to continually accept connections here
        char buffer[4096];
        ssize_t bytes_read, bytes_wrote;
        while (true) {
            if (accept_connection(client_fd_) == 0) {
                log("connection accepted!");

                bytes_read = read(client_fd_, buffer, sizeof(buffer));
                if (bytes_read == -1) {
                    exit_with_error("could not read");
                }

                // get + log request
                std::string raw_request(buffer, bytes_read);
                RequestParams rp(raw_request);
                log("raw request:", raw_request);

                // get + log response
                ResponseParams response(StatusCode::OK, "");
                if (routes_.count(rp.path)) {
                    Handler handler = routes_[rp.path][static_cast<int>(rp.method)];
                    response = std::move(handler(rp));
                }
                log("processed response:", response.response);
                log("end processed response");

                // write to client file descriptor
                bytes_wrote = write(client_fd_, response.response.c_str(), response.response.size());
                log("bytes written", bytes_wrote);
                if (bytes_wrote == -1) {
                    exit_with_error("could not write");
                }
                close(client_fd_);
            }
        }

        return 0;
    }

    int TCPServer::accept_connection(int& new_socket_fd)
    {
        // we don't really care about what the client gives us
        new_socket_fd = accept(socket_fd_, nullptr, nullptr);
        if (new_socket_fd == -1) {
            exit_with_error("connection could not be accepted");
            return -1;
        }
        return 0;
    }

    void TCPServer::close_server()
    {
        if (close(socket_fd_) == -1) {
            exit_with_error("failed to close socket");
            return;
        }
        if (close(client_fd_) == -1) {
            exit_with_error("failed to close socket");
            return;
        }
    }

} // namespace http
