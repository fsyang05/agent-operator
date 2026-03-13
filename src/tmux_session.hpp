#pragma once
#include <string>
#include <vector>
#include <set>

class TmuxSession
{
public:
    TmuxSession(const std::string& session_name = "agent-operator") :
        session_name_(session_name)
    {}
    // automatically called when last window is destroyed
    // kills tmux session
    ~TmuxSession();

    /// Creates window inside of session
    void create_window(const std::string& window_name);

    /// Attaches to window by its name
    void attach_window(const std::string& window_name);

    /// Preview window using tmux's capture-pane, returns captured text
    std::string preview_window(const std::string& window_name);

    /// Kills the window with given name
    void kill_window(const std::string& window_name);

    /// Send keys to a tmux window
    void send_keys(const std::string& window_name, const std::string& keys);

    /// Get vector of all active window names in the session
    std::vector<std::string> list_windows() const;

private:
    std::string session_name_;
    std::set<std::string> window_names_;
    bool initialized_ = false;
    void ensure_session();
    static std::string shell_quote(const std::string& s);
    static std::string popen_read(const std::string& cmd);
};
