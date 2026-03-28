#include <nlohmann/json.hpp>

#include "app/router.hpp"

using namespace http;
using json = nlohmann::json;

namespace
{
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

Router::Router(EventQueue<ServerEvent>& queue, Port port)
    : event_queue_{ queue }
    , server_{ port }
{
    server_.register_route("/hooks/notification", Method::POST,
        [this](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            event_queue_.push(ServerEvent{ ServerEventType::Notification, std::move(sid) });
            return ResponseParams(StatusCode::OK, "");
        });

    server_.register_route("/hooks/stop", Method::POST,
        [this](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            event_queue_.push(ServerEvent{ ServerEventType::Stop, std::move(sid) });
            return ResponseParams(StatusCode::OK, "");
        });

    server_.register_route("/hooks/user-prompt-submit", Method::POST,
        [this](const RequestParams& req) -> ResponseParams {
            auto sid = extract_session_id(req.body);
            event_queue_.push(ServerEvent{ ServerEventType::UserPromptSubmit, std::move(sid) });
            return ResponseParams(StatusCode::OK, "");
        });
}

void Router::run()
{
    server_.start_listen();
}
