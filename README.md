# Agent Operator

A terminal UI for managing multiple Claude Code instances in parallel. Splits your terminal into tiled panes (like tmux), each running an independent Claude Code session. Uses tmux as the process backend and FTXUI for rendering.

## Dependencies

- CMake 3.23+
- C++20 compiler
- tmux
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) (fetched automatically via CMake FetchContent)

## Build & Install

```bash
cmake -B build && cmake --build build
cmake --install build --prefix ~/.local
```

This installs `agent-operator` to `~/.local/bin/`. Make sure `~/.local/bin` is in your `PATH`.

## Keybindings

| Key     | Action                        |
|---------|-------------------------------|
| `v`     | Split pane vertically         |
| `s`     | Split pane horizontally       |
| `d`   | Delete focused pane           |
| `i`   | Enter insert mode (type into pane) |
| `ESC` | Exit insert mode              |
| `RET` | Attach to focused pane        |
| `q`   | Quit                          |

Use arrow keys or `hjkl` (in normal mode) to navigate between panes.
