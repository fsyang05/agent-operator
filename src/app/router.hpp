#pragma once

#include "app/event.hpp"
#include "server/http_tcp_server.hpp"

using Port = unsigned int;

class Router
{
public:
    Router(EventQueue<ServerEvent>& queue, Port port);

    // Starts server listening. Blocks.
    void run();

private:
    EventQueue<ServerEvent>& event_queue_;
    http::TCPServer server_;
};
