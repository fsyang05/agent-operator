#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "app/event.hpp"
#include "model/agent.hpp"

class TUI
{
public:
    explicit TUI(EventQueue<TUIEvent>& queue);

    // Blocks on FTXUI event loop.
    void run();

    // Called by App after state change. Copies agent view data and triggers re-render.
    void update(const std::vector<std::unique_ptr<agent::Agent>>& agents);

    // Thread-safe: triggers FTXUI redraw.
    void post_event();

private:
    EventQueue<TUIEvent>& event_queue_;
    ftxui::ScreenInteractive screen_;

    // Local copy of agent state for rendering
    struct AgentView
    {
        std::string name;
        agent::AgentState state;
        std::string preview;
    };

    std::mutex render_mutex_;
    std::vector<AgentView> agent_views_;
    int selected_index_ = 0;
    bool grid_mode_ = false;

    ftxui::Component build_ui();
    ftxui::Element render_normal_view();
    ftxui::Element render_grid_view();
    ftxui::Element render_help_bar();
};
