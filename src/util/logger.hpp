#include <iostream>
#include <fstream>

#ifdef ENABLE_DEBUG
#define LOG(...) Logger::instance().log(__VA_ARGS__)
#else
#define LOG(...) ((void)0)
#endif

class Logger
{
public:
    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    void set_logfile_dir(const std::string& path)
    {
        path_ = path;
        file_ = std::ofstream(path, std::ios::app);
    }

    template <typename... Args>
    void log(const Args&... args)
    {
        if (file_.is_open()) {
            ((file_ << args << ' '), ...) << std::endl;
        } else {
            std::cout << "log file is not open!" << std::endl;
            return;
        }
    }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    std::ofstream file_;
    std::string path_;
};
