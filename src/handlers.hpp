#pragma once

#include <string>
#include <unordered_map>

#include "model/agent.hpp"

namespace RouteHandler
{
    // session_id -> Agent* mapping (non-owning)
    inline std::unordered_map<std::string, agent::Agent*> session_agent_map;

    inline void on_notification(const std::string& session_id)
    {
        auto it = session_agent_map.find(session_id);
        if (it == session_agent_map.end()) return;
        auto* agent = it->second;
        agent->change_state(agent::AgentState::AGENT_PERMISSION_REQUIRED);
    }

    inline void on_stop(const std::string& session_id)
    {
        auto it = session_agent_map.find(session_id);
        if (it == session_agent_map.end()) return;
        auto* agent = it->second;
        agent->change_state(agent::AgentState::AGENT_IDLE);
    }

    inline void on_user_prompt_submit(const std::string& session_id)
    {
        auto it = session_agent_map.find(session_id);
        if (it == session_agent_map.end()) return;
        auto* agent = it->second;
        agent->change_state(agent::AgentState::AGENT_RUNNING);
    }
}
