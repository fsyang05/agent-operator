#include <stdexcept>
#include <cstdlib>

#include "tmux_session.hpp"

TmuxSession::~TmuxSession()
{
    if (!initialized_) return;
    system(("tmux kill-session -t " + session_name_).c_str());
}

void TmuxSession::ensure_session()
{
    if (initialized_) return;
    // -d runs in background, doesn't immediately attach
    system(("tmux new-session -d -s " + session_name_).c_str());
    initialized_ = true;
}

void TmuxSession::create_window(const std::string& window_name)
{
    ensure_session();
    if (window_names_.count(window_name)) {
        throw std::invalid_argument("Window with window_name: " + window_name + " already exists");
    }
    // tmux new-window -t session_name: -n window_name
    system(("tmux new-window -t " + session_name_ + ": -n " + window_name).c_str());
    window_names_.insert(window_name);
}

void TmuxSession::kill_window(const std::string& window_name)
{
    if (!window_names_.count(window_name)) {
        throw std::invalid_argument("No such window: " + window_name);
    }
    // tmux kill-window -t session_name:window_name
    system(("tmux kill-window -t " + session_name_ + ":" + window_name).c_str());
    window_names_.erase(window_name);
}

std::vector<std::string> TmuxSession::list_windows() const
{
    return { window_names_.begin(), window_names_.end() };
}
