#pragma once
#include <string>
#include <unordered_map>
#include "agent.hpp"

namespace Handler {
    // session_id -> Agent* mapping (non-owning)
    extern std::unordered_map<std::string, agent::Agent*> session_agent_map;

    void register_agent(agent::Agent& agent);
    void remove_agent(agent::Agent& agent);

    // HTTP hook handlers
    void on_notification(const std::string& session_id);
    void on_stop(const std::string& session_id);
    void on_user_prompt_submit(const std::string& session_id);

    // JSON utility
    std::string extract_session_id(const std::string& body);
}
