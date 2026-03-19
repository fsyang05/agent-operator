#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <map>

#include "server/http_tcp_server.hpp"
#include "model/agent.hpp"
#include "model/tmux_session.hpp"

using TMUX = std::shared_ptr<TmuxSession>;

using AgentList = std::vector<std::shared_ptr<agent::Agent>>;
using SessionAgentMap = std::map<agent::SessionId, std::shared_ptr<agent::Agent>>;

class App
{
public:
    std::atomic<bool> running;

    void config_server(http::TCPServer& server);
    void run_server(http::TCPServer& server)
    {
        std::thread server_thread([&server] {
            server.start_listen();
        });
        server_thread.detach();
    }

    template<typename F>
    auto with_agents(F&& fn)
    {
        std::lock_guard lock(mutex_);
        return fn(agents_);
    }
    template<typename F>
    auto with_map(F&& fn)
    {
        std::lock_guard lock(mutex_);
        return fn(map_);
    }

    void create_agent(int new_agent_idx);
    void delete_agent(int agent_idx);
    void attach_agent(int agent_idx);
    void preview_agent(int agent_idx);

    void update_agent(const agent::SessionId& sid, agent::AgentState as);

private:
    TMUX tmux_;

    std::mutex mutex_;

    AgentList agents_;

    SessionAgentMap map_;
};
