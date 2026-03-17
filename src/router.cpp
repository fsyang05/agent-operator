#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>

#include "router.hpp"
#include "handlers.hpp"

using namespace http;

namespace {
    std::string extract_session_id(const std::string& body)
    {
        std::string key = "\"session_id\":\"";
        auto pos = body.find(key);
        if (pos == std::string::npos) {
            // try with space after colon
            key = "\"session_id\": \"";
            pos = body.find(key);
        }
        if (pos == std::string::npos) return "";

        auto start = pos + key.size();
        auto end = body.find('"', start);
        if (end == std::string::npos) return "";
        return body.substr(start, end - start);
    }
}

TCPServer start_server(ftxui::ScreenInteractive& screen, Port port)
{
    TCPServer server(port);

    server.register_route("/hooks/notification", Method::POST,
        [&](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            HTTPHandlers::on_notification(sid);
            screen.PostEvent(ftxui::Event::Custom);
            return ResponseParams(StatusCode::OK, "");
        });

    server.register_route("/hooks/stop", Method::POST,
        [&](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            HTTPHandlers::on_stop(sid);
            screen.PostEvent(ftxui::Event::Custom);
            return ResponseParams(StatusCode::OK, "");
        });

    server.register_route("/hooks/user-prompt-submit", Method::POST,
        [&](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            HTTPHandlers::on_user_prompt_submit(sid);
            screen.PostEvent(ftxui::Event::Custom);
            return ResponseParams(StatusCode::OK, "");
        });

    std::thread server_thread([&server] {
        server.start_listen();
    });
    server_thread.detach();

    return server;
}
