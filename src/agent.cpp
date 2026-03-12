#include "agent.hpp"
#include "util/logger.hpp"

namespace agent
{

    Agent::Agent(std::shared_ptr<TmuxSession> ts, const std::string& window_name, const std::string& agent_name)
    {
        LOG("(AGENT) creating new agent with name", agent_name, "window name", window_name);
        this->agent_name = std::move(agent_name);
        window_name_ = std::move(window_name);
        tmux_session_ = std::move(ts);
        state = AgentState::AGENT_IDLE;

        // we should attach to the session, and then run claude in the background
        LOG("(AGENT) creating window for agent", agent_name, "window name", window_name_);
        tmux_session_->create_window(window_name_);
    }

    Agent::~Agent()
    {
        LOG("(AGENT) destroying window for agent", agent_name, "window name", window_name_);
        tmux_session_->kill_window(window_name_);
    }

    void Agent::attach()
    {
        LOG("(AGENT) attaching window for agent", agent_name, "window name", window_name_);
        tmux_session_->attach_window(window_name_);
    }

    std::string Agent::get_preview()
    {
        LOG("(AGENT) getting preview for agent", agent_name, "window name", window_name_);
        return tmux_session_->preview_window(window_name_);
    }

} // namespace Agent
