#pragma once

#include <string>

#include "util/logger.hpp"

namespace agent
{

    using SessionId = std::string;
    class Agent
    {
    public:
        Agent(std::string name, SessionId sid)
        : name_{ std::move(name) }
        , session_id_{ std::move(sid) }
        {}

        ~Agent()
        {
            LOG("(AGENT) destroying agent", name_);
        }

        const SessionId& session_id() const { return session_id_; }
        const std::string& name() const { return name_; }

        const std::string& preview() const { return preview_; }
        void set_preview(std::string p) { preview_ = std::move(p); }

    private:
        std::string name_;
        SessionId session_id_;
        std::string preview_;
    };

} // namespace agent
