#pragma once
#include <string>
#include <cstdlib>

// from the hooks documentation
// can be actively running,
// waiting for some decision from the user,
// completed its task,
// etc.
//
// i think it is enough to just have:
// idle
// running
// permission required
// stopped (maybe don't even need this)
enum class AgentState {
    AGENT_RUNNING,
    AGENT_IDLE,
    AGENT_PERMISSION_REQUIRED,
    AGENT_STOPPED
};

/// An abstraction of a running Claude Code instance.
/// Supports starting a new instance, ending an instance.
class Agent
{
public:
    Agent(const std::string& agent_name)
    : agent_name_(agent_name)
    {
        state_ = AgentState::AGENT_IDLE;
        system(("claude --agent " + agent_name_).c_str());
    }

    ~Agent() {}

private:
    AgentState state_; // state transitions should be handled internally
    std::string agent_name_; // start with claude --agent <name>
    std::string session_id_; // lowkey probably won't use this then
    std::string transcript_path_;
};
