#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "app/event.hpp"
#include "model/agent.hpp"
#include "model/tmux_session.hpp"

class TUI;

class App
{
public:
    enum class Action { Quit, Attach };

    App();

    void run();

    Action last_action() const { return last_action_; }
    int attach_pane_id() const { return attach_pane_id_; }

    void attach_agent(int pane_id);
    void reset_for_reentry() { running_ = true; }

    void set_tui(TUI& tui) { tui_ = &tui; }
    EventQueue<TUIEvent>& tui_queue() { return tui_queue_; }

private:
    std::unique_ptr<Tmux::TmuxSession> tmux_;
    std::map<int, std::unique_ptr<agent::Agent>> agents_;  // keyed by pane_id
    int next_agent_id_ = 0;
    std::atomic<bool> running_{ true };

    Action last_action_ = Action::Quit;
    int attach_pane_id_ = -1;

    EventQueue<TUIEvent> tui_queue_;
    TUI* tui_ = nullptr;

    void handle_tui_event(TUIEvent e);

    void create_agent(int pane_id);
    void delete_agent(int pane_id);
    void start_agent(int pane_id);
    void send_keys_to_agent(int pane_id, const std::string& keys);
    void send_special_key_to_agent(int pane_id, const std::string& key_name);

    void update_tui();
    void poll_previews();
};
