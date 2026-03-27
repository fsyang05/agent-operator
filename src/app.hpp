#pragma once

#include <mutex>
#include <vector>
#include <memory>

#include "server/http_tcp_server.hpp"
#include "model/agent.hpp"
#include "model/tmux_session.hpp"

struct AgentInfo
{
    agent::SessionId sid;
    std::unique_ptr<agent::Agent> agent;
};

class App
{
public:
    App()
    : tmux_{ std::make_unique<TmuxSession>() }
    {}

    std::atomic<bool> running;

    void config_server(http::TCPServer& server);
    void run_server(http::TCPServer& server);

    template<typename F>
    auto with_agents(F&& fn);

    template<typename F>
    auto with_map(F&& fn);

    void create_agent(int new_agent_idx);
    void delete_agent();
    void attach_agent();
    void preview_agent();

    void update_agent(const agent::SessionId& sid, agent::AgentState as);

private:
    int selected_idx_;

    std::unique_ptr<TmuxSession> tmux_;

    std::mutex mutex_;

    std::vector<AgentInfo> agents_;
};
