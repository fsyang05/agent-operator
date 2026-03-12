#pragma once

#include <netinet/in.h>
#include <map>
#include <array>
#include <string>
#include <functional>

namespace http
{
    enum class Method
    {
        GET,
        POST,
        NUM_METHODS
    }; // enum class Method

    enum class StatusCode
    {
        OK          = 200,
        NOT_FOUND   = 404
    }; // enum class StatusCode

    struct RequestParams
    {
        RequestParams(const std::string& raw_request);

        Method method;
        std::string path;
        std::string body;
    };

    // at minimum, need a status
    // return 200 with empty body for claude
    struct ResponseParams
    {
        ResponseParams(StatusCode status, const std::string& body);

        std::string response;
    };

    using Handler = std::function<ResponseParams(const RequestParams&)>;
    using HandlerArr = std::array<Handler, (size_t)Method::NUM_METHODS>;

    class TCPServer
    {
    public:
        TCPServer(const int& port);
        ~TCPServer();
        void register_route(const std::string& path, Method method, Handler handler);
        int start_listen();

    private:
        // another way to do this:
        // map string -> HandlerArr, just a vector of handlers
        // with fixed size
        std::map<std::string, HandlerArr> routes_;
        int socket_fd_;
        int client_fd_;
        int port_;
        sockaddr_in addr_;
        int start_server();
        int accept_connection(int& new_socket_fd);
        void close_server();
    };

} // namespace http

