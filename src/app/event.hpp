#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

// TUI -> App
enum class TUIEventType
{
    CreateAgent,
    DeleteAgent,
    AttachAgent,
    SendKeys,        // literal text via send-keys -l
    SendSpecialKey,  // tmux key name via send-keys (e.g. "Enter", "BSpace")
    Quit
};

struct TUIEvent
{
    TUIEventType type;
    int pane_id;
    std::string keys;
};

// Thread-safe MPSC queue (mutex + std::queue)
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
