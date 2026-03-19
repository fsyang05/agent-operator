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
        Agent(const std::string& window_name, const std::string& agent_name, const SessionId& sid)
        : window_name_(std::move(window_name)), agent_name_(std::move(agent_name)), session_id_(std::move(sid)) {}

        ~Agent()
        {
            LOG("(AGENT) destroying window for agent", agent_name_, "window name", window_name_);
        }

        const std::string& session_id() const { return session_id_; }
        const std::string& window_name() const { return window_name_; }
        const std::string& agent_name() const { return agent_name_; }
        const AgentState& agent_state() const { return state_; }
        void change_state(const AgentState as) { state_ = as; }

    private:
        AgentState state_;
        std::string window_name_;
        std::string agent_name_;
        std::string session_id_;
    };

} // namespace agent
