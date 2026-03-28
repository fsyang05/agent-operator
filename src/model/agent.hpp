#pragma once

#include <string>

#include "util/logger.hpp"

namespace agent
{

    enum class AgentState
    {
        AGENT_RUNNING,
        AGENT_IDLE,
        AGENT_PERMISSION_REQUIRED,
        AGENT_STOPPED
    };

    inline std::string state_to_str(const AgentState as)
    {
        switch (as) {
            case AgentState::AGENT_RUNNING              :   return "RUNNING";
            case AgentState::AGENT_IDLE                 :   return "IDLE";
            case AgentState::AGENT_PERMISSION_REQUIRED  :   return "PERMISSION_REQUIRED";
            case AgentState::AGENT_STOPPED              :   return "STOPPED";
        }
        return "UNKNOWN";
    }

    using SessionId = std::string;
    class Agent
    {
    public:
        Agent(std::string name, SessionId sid)
        : state_{ AgentState::AGENT_IDLE }
        , name_{ std::move(name) }
        , session_id_{ std::move(sid) }
        {}

        ~Agent()
        {
            LOG("(AGENT) destroying agent", name_);
        }

        const SessionId& session_id() const { return session_id_; }
        const std::string& name() const { return name_; }
        AgentState agent_state() const { return state_; }
        void change_state(AgentState as) { state_ = as; }

        const std::string& preview() const { return preview_; }
        void set_preview(std::string p) { preview_ = std::move(p); }

    private:
        AgentState state_;
        std::string name_;
        SessionId session_id_;
        std::string preview_;
    };

} // namespace agent
