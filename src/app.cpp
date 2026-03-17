#include <random>
#include <sstream>
#include <string>
#include <iomanip>
#include <thread>
#include <stdexcept>

#include "app.hpp"

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

    std::string make_name(const int agent_idx)
    {
        return "agent-" + std::to_string(agent_idx);
    }
}

// for handling server
void App::config_server(http::TCPServer& server)
{

}

void App::run_server(http::TCPServer& server)
{
    std::thread server_thread([&server] {
        server.start_listen();
    });
    server_thread.detach();
}

// locks for using agents + map
template<typename F>
auto App::with_agents(F&& fn)
{
    std::lock_guard lock(mutex_);
    return fn(agents_);
}

// since we're spamming agent methods here
using namespace agent;

/// Updates application's list + map of agents
/// Creates tmux session and agent panel (TODO)
void App::create_agent(int new_agent_idx)
{
    std::string name = make_name(new_agent_idx);
    tmux_->create_window(name);

    const SessionId uuid = generate_uuid();
    agents_.emplace_back(generate_uuid(),
                         std::make_unique<Agent>(name, uuid));
}

/// Deletes selected agent
/// Rebuilds agent container (TODO)
void App::delete_agent()
{
    if (agents_.empty())
        return;
    if (selected_idx_ < 0 || selected_idx_ >= (int)agents_.size())
        throw std::invalid_argument("Index out of bounds.");

    auto it = agents_.begin();
    agents_.erase(it + selected_idx_);
}

/// Attach to the right tmux session
/// TODO: is this thread safe or do i need to fix
void App::attach_agent()
{
    if (agents_.empty())
        return;
    if (selected_idx_ < 0 || selected_idx_ >= (int)agents_.size())
        throw std::invalid_argument("Index out of bounds.");

    const AgentInfo& ai = agents_[selected_idx_];
    tmux_->attach_window(ai.agent->name());
}

/// Get the terminal preview for a tmux session
/// Refreshes the component
void App::preview_agent()
{

}

void update_agent(const agent::SessionId sid, agent::AgentState as)
{

}
