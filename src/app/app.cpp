#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <thread>

#include "app/app.hpp"
#include "ui/tui.hpp"
#include "util/logger.hpp"

namespace
{
    std::string generate_uuid()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

        uint32_t a = dist(gen);
        uint16_t b = dist(gen) & 0xFFFF;
        uint16_t c = (dist(gen) & 0x0FFF) | 0x4000;
        uint16_t d = (dist(gen) & 0x3FFF) | 0x8000;
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

App::App()
    : tmux_{ std::make_unique<Tmux::TmuxSession>() }
{}

void App::run()
{
    auto last_preview = std::chrono::steady_clock::now();

    while (running_) {
        // Drain TUI events
        while (auto ev = tui_queue_.try_pop()) {
            handle_tui_event(*ev);
            if (!running_) return;
        }

        // Drain server events
        while (auto ev = server_queue_.try_pop()) {
            handle_server_event(*ev);
        }

        // Poll previews periodically (all agents_ access stays on this thread)
        auto now = std::chrono::steady_clock::now();
        if (now - last_preview >= std::chrono::milliseconds(100)) {
            last_preview = now;
            poll_previews();
        }

        if (running_)
            update_tui();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void App::handle_tui_event(const TUIEvent& e)
{
    switch (e.type) {
        case TUIEventType::CreateAgent:
            create_agent();
            break;
        case TUIEventType::StartAgent:
            start_agent(e.selected_index);
            break;
        case TUIEventType::DeleteAgent:
            delete_agent(e.selected_index);
            break;
        case TUIEventType::AttachAgent:
            last_action_ = Action::Attach;
            attach_index_ = e.selected_index;
            running_ = false;
            break;
        case TUIEventType::Quit:
            last_action_ = Action::Quit;
            running_ = false;
            break;
    }
}

void App::handle_server_event(const ServerEvent& e)
{
    auto it = std::find_if(agents_.begin(), agents_.end(),
        [&](const auto& a) { return a->session_id() == e.session_id; });
    if (it == agents_.end()) return;

    auto* ag = it->get();
    switch (e.type) {
        case ServerEventType::Notification:
            ag->change_state(agent::AgentState::AGENT_PERMISSION_REQUIRED);
            break;
        case ServerEventType::Stop:
            ag->change_state(agent::AgentState::AGENT_IDLE);
            break;
        case ServerEventType::UserPromptSubmit:
            ag->change_state(agent::AgentState::AGENT_RUNNING);
            break;
    }
    LOG("(APP) server event for session", e.session_id, "new state:", agent::state_to_str(ag->agent_state()));
}

void App::create_agent()
{
    std::string name = "agent-" + std::to_string(next_agent_id_++);
    tmux_->create_window(name);
    auto sid = generate_uuid();
    agents_.push_back(std::make_unique<agent::Agent>(name, sid));
    LOG("(APP) created agent", name);
}

void App::delete_agent(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(agents_.size())) return;

    auto& ag = agents_[idx];
    tmux_->kill_window(ag->name());
    LOG("(APP) deleted agent", ag->name());
    agents_.erase(agents_.begin() + idx);
}

void App::start_agent(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(agents_.size())) return;

    auto& ag = agents_[idx];
    if (ag->agent_state() != agent::AgentState::AGENT_IDLE) return;

    tmux_->send_keys(ag->name(), "claude\n");
    ag->change_state(agent::AgentState::AGENT_RUNNING);
    LOG("(APP) started agent", ag->name(), "session", ag->session_id());
}

void App::attach_agent(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(agents_.size())) return;

    tmux_->attach_window(agents_[idx]->name());
    // After returning from tmux attach, TUI re-enters its loop via main.cpp
}

void App::update_tui()
{
    if (tui_)
        tui_->update(agents_);
}

void App::poll_previews()
{
    for (auto& ag : agents_) {
        auto preview = tmux_->preview_window(ag->name());
        if (preview != ag->preview()) {
            ag->set_preview(std::move(preview));
        }
    }
}
