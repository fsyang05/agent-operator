#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <variant>

// TUI → App
enum class TUIEventType
{
    CreateAgent,
    StartAgent,
    DeleteAgent,
    AttachAgent,
    Quit
};

struct TUIEvent
{
    TUIEventType type;
    int selected_index;
};

// Server → App
enum class ServerEventType
{
    Notification,
    Stop,
    UserPromptSubmit
};

struct ServerEvent
{
    ServerEventType type;
    std::string session_id;
};

using AppEvent = std::variant<TUIEvent, ServerEvent>;

// Simple SPSC queue (mutex + std::queue)
template<typename T>
class EventQueue
{
public:
    void push(T event)
    {
        std::lock_guard lock(mutex_);
        queue_.push(std::move(event));
        cv_.notify_one();
    }

    std::optional<T> try_pop()
    {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) return std::nullopt;
        T val = std::move(queue_.front());
        queue_.pop();
        return val;
    }

    T wait_pop()
    {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T val = std::move(queue_.front());
        queue_.pop();
        return val;
    }

    void notify()
    {
        cv_.notify_one();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
