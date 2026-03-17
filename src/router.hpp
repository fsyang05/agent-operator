#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "server/http_tcp_server.hpp"

using Port = int;

http::TCPServer start_server(ftxui::ScreenInteractive& screen, Port port);
