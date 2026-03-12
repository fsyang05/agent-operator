#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <array>

#include "tmux_session.hpp"
#include "util/logger.hpp"

std::string TmuxSession::shell_quote(const std::string& s)
{
    std::string result = "'";
    for (char c : s) {
        if (c == '\'')
            result += "'\\''";
        else
            result += c;
    }
    result += "'";
    return result;
}

std::string TmuxSession::popen_read(const std::string& cmd)
{
    std::string output;
    std::array<char, 4096> buf;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return output;
    while (fgets(buf.data(), buf.size(), pipe) != nullptr) {
        output += buf.data();
    }
    pclose(pipe);
    return output;
}

TmuxSession::~TmuxSession()
{
    LOG("(TMUX) killing session with name", session_name_);

    if (!initialized_) return;
    std::string cmd = "tmux kill-session -t " + shell_quote(session_name_);
    system(cmd.c_str());
}

void TmuxSession::ensure_session()
{
    if (initialized_) return;
    // -d runs in background, doesn't immediately attach
    std::string cmd = "tmux new-session -d -s " + shell_quote(session_name_);
    int rc = system(cmd.c_str());
    if (rc != 0) {
        throw std::runtime_error("Failed to create tmux session: " + session_name_);
    }
    initialized_ = true;
}

void TmuxSession::create_window(const std::string& window_name)
{
    LOG("(TMUX) attempting to create window with name", window_name);

    ensure_session();
    if (window_names_.count(window_name)) {
        throw std::invalid_argument("Window with window_name: " + window_name + " already exists");
    }
    // tmux new-window -t session_name: -n window_name
    std::string cmd = "tmux new-window -t " + shell_quote(session_name_) + ": -n " + shell_quote(window_name);
    int rc = system(cmd.c_str());
    if (rc != 0) {
        throw std::runtime_error("Failed to create tmux window: " + window_name);
    }
    window_names_.insert(window_name);

    LOG("(TMUX) created window with name", window_name);
}

void TmuxSession::attach_window(const std::string& window_name)
{
    LOG("(TMUX) attempting to attach to window with name", window_name);

    ensure_session();
    if (!window_names_.count(window_name)) {
        throw std::invalid_argument("Window with window_name: " + window_name + " doesn't exist");
    }
    // tmux attach-session -t session_name:window_name
    std::string cmd = "tmux attach-session -t " + shell_quote(session_name_ + ":" + window_name);
    system(cmd.c_str());

    LOG("(TMUX) attached to window with name", window_name);
}

std::string TmuxSession::preview_window(const std::string& window_name)
{
    LOG("(TMUX) attempting to preview window with name", window_name);

    ensure_session();
    if (!window_names_.count(window_name)) {
        throw std::invalid_argument("Window with window_name: " + window_name + " doesn't exist");
    }
    // tmux capture-pane -t session_name:window_name -p
    std::string cmd = "tmux capture-pane -t " + shell_quote(session_name_ + ":" + window_name) + " -p";

    LOG("(TMUX) previewed window with name", window_name);
    return popen_read(cmd);
}

void TmuxSession::kill_window(const std::string& window_name)
{
    LOG("(TMUX) attempting to kill window with name", window_name);

    ensure_session();
    if (!window_names_.count(window_name)) {
        throw std::invalid_argument("No such window: " + window_name);
    }
    // tmux kill-window -t session_name:window_name
    std::string cmd = "tmux kill-window -t " + shell_quote(session_name_ + ":" + window_name);
    int rc = system(cmd.c_str());
    if (rc != 0) {
        throw std::runtime_error("Failed to kill tmux window: " + window_name);
    }
    window_names_.erase(window_name);

    LOG("(TMUX) killed window with name", window_name);
}

std::vector<std::string> TmuxSession::list_windows() const
{
    return { window_names_.begin(), window_names_.end() };
}
