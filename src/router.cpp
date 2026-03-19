#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

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

void config_server(TCPServer& server, ftxui::ScreenInteractive& screen)
{
    server.register_route("/hooks/notification", Method::POST,
        [&](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            RouteHandler::on_notification(sid);
            screen.PostEvent(ftxui::Event::Custom);
            return ResponseParams(StatusCode::OK, "");
        });

    server.register_route("/hooks/stop", Method::POST,
        [&](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            RouteHandler::on_stop(sid);
            screen.PostEvent(ftxui::Event::Custom);
            return ResponseParams(StatusCode::OK, "");
        });

    server.register_route("/hooks/user-prompt-submit", Method::POST,
        [&](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            RouteHandler::on_user_prompt_submit(sid);
            screen.PostEvent(ftxui::Event::Custom);
            return ResponseParams(StatusCode::OK, "");
        });
}
