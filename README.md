# Agent Operator

C++ TUI for managing multiple Claude Code instances in parallel. Uses tmux as the process host and FTXUI for terminal rendering.

## Threading

Three threads run concurrently:

- **Main thread** — FTXUI event loop. Handles keyboard input and renders the UI.
- **HTTP server thread** — A `std::thread` (detached) runs a raw TCP socket server using POSIX C APIs (`socket`, `bind`, `listen`, `accept`, `read`, `write`). It receives POST callbacks from Claude Code hooks (`/hooks/notification`, `/hooks/stop`, `/hooks/user-prompt-submit`) and posts `Event::Custom` to trigger UI redraws.
- **Polling thread** — A `std::thread` (detached) sleeps for 500ms in a loop and posts `Event::Custom` to keep terminal previews refreshed with live tmux pane captures.

## Build & Run

```bash
cmake -B build && cmake --build build
./build/run
```

Requires CMake 3.23+, C++20 compiler. FTXUI is fetched via CMake FetchContent.
