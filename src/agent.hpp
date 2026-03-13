#pragma once

#include <string>
#include <memory>
#include <cstdlib>
#include <random>
#include <sstream>
#include <iomanip>

#include <ftxui/screen/color.hpp>

#include "tmux_session.hpp"

/// An abstraction of a running Claude Code instance.
/// Supports starting a new instance, ending an instance.
namespace agent
{

    enum class AgentState {
        AGENT_RUNNING,
        AGENT_IDLE,
        AGENT_PERMISSION_REQUIRED,
        AGENT_STOPPED
    };

    constexpr const char* state_to_str(AgentState as)
    {
        switch (as) {
            case AgentState::AGENT_RUNNING              :   return "RUNNING";
            case AgentState::AGENT_IDLE                 :   return "IDLE";
            case AgentState::AGENT_PERMISSION_REQUIRED  :   return "PERMISSION_REQUIRED";
            case AgentState::AGENT_STOPPED              :   return "STOPPED";
        }
        return "UNKNOWN";
    }

    inline ftxui::Color state_to_color(AgentState as)
    {
        switch (as) {
            case AgentState::AGENT_RUNNING              :   return ftxui::Color::Green;
            case AgentState::AGENT_IDLE                 :   return ftxui::Color::Yellow;
            case AgentState::AGENT_PERMISSION_REQUIRED  :   return ftxui::Color::Red;
            case AgentState::AGENT_STOPPED              :   return ftxui::Color::GrayDark;
        }
        return ftxui::Color::White;
    }

    class Agent
    {
    public:
        Agent(std::shared_ptr<TmuxSession> ts, const std::string& window_name, const std::string& agent_name);
        ~Agent();

        AgentState state;
        std::string agent_name;

        void attach();
        std::string get_preview();
        void start_claude();
        void stop_claude();
        const std::string& session_id() const { return session_id_; }

    private:
        std::shared_ptr<TmuxSession> tmux_session_;
        std::string session_id_;
        std::string window_name_;
        std::string transcript_path_;
    };

} // namespace agent
