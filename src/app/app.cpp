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
        while (auto ev = tui_queue_.try_pop()) {
            handle_tui_event(std::move(*ev));
            if (!running_) return;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - last_preview >= std::chrono::milliseconds(100)) {
            last_preview = now;
            poll_previews();
            if (running_)
                update_tui();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void App::handle_tui_event(TUIEvent e)
{
    switch (e.type) {
        case TUIEventType::CreateAgent:
            create_agent(e.pane_id);
            break;
        case TUIEventType::StartAgent:
            start_agent(e.pane_id);
            break;
        case TUIEventType::DeleteAgent:
            delete_agent(e.pane_id);
            break;
        case TUIEventType::AttachAgent:
            last_action_ = Action::Attach;
            attach_pane_id_ = e.pane_id;
            running_ = false;
            break;
        case TUIEventType::SendKeys:
            send_keys_to_agent(e.pane_id, e.keys);
            break;
        case TUIEventType::SendSpecialKey:
            send_special_key_to_agent(e.pane_id, e.keys);
            break;
        case TUIEventType::Quit:
            last_action_ = Action::Quit;
            running_ = false;
            break;
    }
}

void App::create_agent(int pane_id)
{
    std::string name = "agent-" + std::to_string(next_agent_id_++);
    tmux_->create_window(name);
    auto sid = generate_uuid();
    agents_.emplace(pane_id, std::make_unique<agent::Agent>(name, sid));
    LOG("(APP) created agent", name, "pane", pane_id);
}

void App::delete_agent(int pane_id)
{
    auto it = agents_.find(pane_id);
    if (it == agents_.end()) return;

    tmux_->kill_window(it->second->name());
    LOG("(APP) deleted agent", it->second->name(), "pane", pane_id);
    agents_.erase(it);
}

void App::start_agent(int pane_id)
{
    auto it = agents_.find(pane_id);
    if (it == agents_.end()) return;

    auto& ag = it->second;
    if (ag->agent_state() != agent::AgentState::AGENT_IDLE) return;

    tmux_->send_keys(ag->name(), "claude");
    ag->change_state(agent::AgentState::AGENT_RUNNING);
    LOG("(APP) started agent", ag->name(), "session", ag->session_id());
}

void App::send_keys_to_agent(int pane_id, const std::string& keys)
{
    auto it = agents_.find(pane_id);
    if (it == agents_.end()) return;
    tmux_->send_keys_raw(it->second->name(), keys);
}

void App::send_special_key_to_agent(int pane_id, const std::string& key_name)
{
    auto it = agents_.find(pane_id);
    if (it == agents_.end()) return;
    tmux_->send_special_key(it->second->name(), key_name);
}

void App::attach_agent(int pane_id)
{
    auto it = agents_.find(pane_id);
    if (it == agents_.end()) return;
    tmux_->attach_window(it->second->name());
}

void App::update_tui()
{
    if (tui_)
        tui_->update(agents_);
}

void App::poll_previews()
{
    for (auto& [id, ag] : agents_) {
        auto preview = tmux_->preview_window(ag->name());
        if (preview != ag->preview()) {
            ag->set_preview(std::move(preview));
        }
    }
}
