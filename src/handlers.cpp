#include "handlers.hpp"
#include "util/logger.hpp"

namespace HTTPHandlers {

    std::unordered_map<std::string, agent::Agent*> session_agent_map; 

    void on_notification(const std::string& session_id)
    {
        auto it = session_agent_map.find(session_id);
        if (it == session_agent_map.end()) return;
        auto* agent = it->second;
        agent->state = agent::AgentState::AGENT_PERMISSION_REQUIRED;
        LOG("(HANDLER) notification -> PERMISSION_REQUIRED for agent", agent->agent_name);
    }

    void on_stop(const std::string& session_id)
    {
        auto it = session_agent_map.find(session_id);
        if (it == session_agent_map.end()) return;
        auto* agent = it->second;
        agent->state = agent::AgentState::AGENT_IDLE;
        LOG("(HANDLER) stop -> IDLE for agent", agent->agent_name);
    }

    void on_user_prompt_submit(const std::string& session_id)
    {
        auto it = session_agent_map.find(session_id);
        if (it == session_agent_map.end()) return;
        auto* agent = it->second;
        agent->state = agent::AgentState::AGENT_RUNNING;
        LOG("(HANDLER) user_prompt_submit -> RUNNING for agent", agent->agent_name);
    }

} // namespace Handler
