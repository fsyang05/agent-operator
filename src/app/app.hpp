#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "app/event.hpp"
#include "model/agent.hpp"
#include "model/tmux_session.hpp"

class TUI;

class App
{
public:
    enum class Action { Quit, Attach };

    App();

    // Main event loop — blocks until Quit or Attach.
    void run();

    // Call after run() returns to check what happened.
    Action last_action() const { return last_action_; }
    int attach_index() const { return attach_index_; }

    // Attach to tmux window. Call from main thread after TUI has fully exited.
    void attach_agent(int idx);

    // Reset so run() can be called again after an attach.
    void reset_for_reentry() { running_ = true; }

    void set_tui(TUI* tui) { tui_ = tui; }

    EventQueue<TUIEvent>& tui_queue() { return tui_queue_; }
    EventQueue<ServerEvent>& server_queue() { return server_queue_; }

private:
    std::unique_ptr<Tmux::TmuxSession> tmux_;
    std::vector<std::unique_ptr<agent::Agent>> agents_;
    std::unordered_map<std::string, agent::Agent*> session_map_;
    int next_agent_id_ = 0;
    std::atomic<bool> running_{ true };

    Action last_action_ = Action::Quit;
    int attach_index_ = -1;

    EventQueue<TUIEvent> tui_queue_;
    EventQueue<ServerEvent> server_queue_;
    TUI* tui_ = nullptr;

    void handle_tui_event(const TUIEvent& e);
    void handle_server_event(const ServerEvent& e);

    void create_agent();
    void delete_agent(int idx);
    void start_agent(int idx);

    void update_tui();
    void poll_previews();
};
