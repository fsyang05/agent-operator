#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <nlohmann/json.hpp>

#include "router.hpp"
#include "handlers.hpp"

using namespace http;
using json = nlohmann::json;

namespace {
    std::string extract_session_id(const std::string& body)
    {
        try {
            auto j = json::parse(body);
            return j.at("session_id").get<std::string>();
        } catch (const json::exception&) {
            return "";
        }
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
