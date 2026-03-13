#include "handlers.hpp"
#include "util/logger.hpp"

namespace Handler {

    std::unordered_map<std::string, agent::Agent*> session_agent_map;

    void register_agent(agent::Agent& agent)
    {
        session_agent_map[agent.session_id()] = &agent;
        LOG("(HANDLER) registered session", agent.session_id(), "for agent", agent.agent_name);
    }

    void remove_agent(agent::Agent& agent)
    {
        session_agent_map.erase(agent.session_id());
        LOG("(HANDLER) removed agent from session map", agent.agent_name);
    }

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

} // namespace Handler
