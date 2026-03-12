#include "http_tcp_server.hpp"

using namespace http;

ResponseParams handler(const RequestParams& rp)
{
    ResponseParams res(StatusCode::OK, rp.body);
    return res;
}

int main()
{
    TCPServer server(8080);
    server.register_route("/test/path/", Method::GET, handler);
    server.register_route("/test/path/", Method::POST, handler);
    server.start_listen();
    return 0;
}
