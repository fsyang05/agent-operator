#include <random>
#include <sstream>
#include <iomanip>

#include "agent.hpp"
#include "util/logger.hpp"

namespace
{
    static std::string generate_uuid()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

        uint32_t a = dist(gen);
        uint16_t b = dist(gen) & 0xFFFF;
        uint16_t c = (dist(gen) & 0x0FFF) | 0x4000; // version 4
        uint16_t d = (dist(gen) & 0x3FFF) | 0x8000; // variant 1
        uint32_t e1 = dist(gen);
        uint16_t e2 = dist(gen) & 0xFFFF;

        std::ostringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(8) << a << '-'
           << std::setw(4) << b << '-'
           << std::setw(4) << c << '-'
           << std::setw(4) << d << '-'
           << std::setw(8) << e1
           << std::setw(4) << e2;
        return ss.str();
    }
}

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

    void Agent::start_claude()
    {
        LOG("(AGENT) starting claude for agent", agent_name);
        session_id_ = generate_uuid();
        std::string cmd = "claude --session-id " + session_id_;
        tmux_session_->send_keys(window_name_, cmd);
        LOG("(AGENT) started claude for agent", agent_name, "session_id", session_id_);
    }

    void Agent::stop_claude()
    {
        LOG("(AGENT) stopping claude for agent", agent_name);
        // Send C-c without Enter — use raw tmux key name
        std::string cmd = "tmux send-keys -t " + window_name_ + " C-c";
        system(cmd.c_str());
        state = AgentState::AGENT_STOPPED;
        LOG("(AGENT) stopped claude for agent", agent_name);
    }

} // namespace Agent
