#pragma once

#include <string>
#include <vector>
#include <set>

class TmuxSession
{
    public:
        TmuxSession(const std::string& session_name = "agent-operator") :
            session_name_{std::move(session_name)}, initialized_{false}
            {}
        ~TmuxSession();

        void create_window(const std::string& window_name);
        void attach_window(const std::string& window_name);
        std::string preview_window(const std::string& window_name);
        void kill_window(const std::string& window_name);
        void send_keys(const std::string& window_name, const std::string& keys);
        std::vector<std::string> list_windows() const;

    private:
        std::string session_name_;
        std::set<std::string> window_names_;
        bool initialized_;
        void ensure_session();
        static std::string shell_quote(const std::string& s);
        static std::string popen_read(const std::string& cmd);
};
