#include <atomic>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "model/agent.hpp"
#include "model/tmux_session.hpp"
#include "router.cpp"
#include "server/http_tcp_server.hpp"
#include "ui/ansi_parser.hpp"
#include "ui/components.hpp"
#include "util/logger.hpp"

using namespace ftxui;
using namespace agent;

namespace {

    void register_agent(agent::Agent& agent)
    {
        HTTPHandlers::session_agent_map[agent.session_id()] = &agent;
        LOG("(HANDLER) registered session", agent.session_id(), "for agent", agent.agent_name);
    }

    void remove_agent(agent::Agent& agent)
    {
        HTTPHandlers::session_agent_map.erase(agent.session_id());
        LOG("(HANDLER) removed agent from session map", agent.agent_name);
    }

}

int main() {
  try {
    // setup logger
    Logger::instance().set_logfile_dir("app.log");

    auto screen = ScreenInteractive::Fullscreen();

    // HTTP server for Claude Code hook callbacks
    http::TCPServer server = start_server(screen, 8080);

    auto tmux = std::make_shared<TmuxSession>();
    std::vector<std::unique_ptr<Agent>> agents;

    int selected_agent_index = 0;
    int next_agent_id = 0;
    auto agent_panel_container = Container::Vertical({}, &selected_agent_index);

    std::string preview_text;
    Component preview_component = Renderer([&] {
        return text("No agent selected") | dim | center;
    });

    auto agent_panel_build_container = [&] {
        agent_panel_container->DetachAllChildren();
        for (auto& a : agents) {
            agent_panel_container->Add(AgentPanel(*a));
        }
        if (selected_agent_index >= (int)agents.size())
            selected_agent_index = std::max(0, (int)agents.size() - 1);
    };

    auto agent_create = [&] {
        std::string name = "agent-" + std::to_string(next_agent_id++);
        agents.push_back(std::make_unique<Agent>(tmux, name, name));
        agent_panel_build_container();
    };

    auto agent_kill = [&] {
        if (agents.empty()) return;
        int idx = selected_agent_index;
        if (idx >= 0 && idx < (int)agents.size()) {
            remove_agent(*agents[idx]);
            agents.erase(agents.begin() + idx);
            agent_panel_build_container();
        }
    };

    auto agent_start = [&] {
        if (agents.empty()) return;
        int idx = selected_agent_index;
        if (idx >= 0 && idx < (int)agents.size()) {
            auto& agent = *agents[idx];
            if (agent.state != AgentState::AGENT_IDLE) return;
            agent.start_claude();
            register_agent(agent);
        }
    };

    bool grid_mode = false;

    auto renderer = Renderer(agent_panel_container, [&] {
        // help text with highlighted keys
        auto help = hbox({
            text("n") | bold | color(Color::BlueLight), text(":spawn ") | dim,
            text("s") | bold | color(Color::BlueLight), text(":start ") | dim,
            text("d") | bold | color(Color::BlueLight), text(":kill ") | dim,
            text("ret") | bold | color(Color::BlueLight), text(":attach ") | dim,
            text("g") | bold | color(Color::BlueLight), text(":grid ") | dim,
            text("q") | bold | color(Color::BlueLight), text(":quit") | dim,
        });

        if (grid_mode) {
            // Grid view
            constexpr int cols = 3;
            int n = (int)agents.size();

            Element grid_content;
            if (n == 0) {
                grid_content = text("No agents") | dim | center | flex;
            } else {
                std::vector<Elements> grid_rows;
                Elements current_row;
                for (int i = 0; i < n; i++) {
                    auto& a = *agents[i];
                    Color sc = state_to_color(a.state);
                    std::string preview = a.get_preview();

                    auto cell = vbox({
                        hbox({
                            text(a.agent_name) | bold,
                            text(" "),
                            text(state_to_str(a.state)) | color(sc) | bold,
                        }),
                        separator(),
                        ansi::ansi_to_element(preview) | flex,
                    }) | borderStyled(sc) | flex;

                    current_row.push_back(cell);
                    if ((int)current_row.size() == cols) {
                        grid_rows.push_back(std::move(current_row));
                        current_row.clear();
                    }
                }
                // pad last row with empty flex cells
                while ((int)current_row.size() < cols) {
                    current_row.push_back(vbox({}) | flex);
                }
                if (!current_row.empty()) {
                    grid_rows.push_back(std::move(current_row));
                }
                grid_content = gridbox(std::move(grid_rows)) | flex;
            }

            return vbox({
                grid_content | flex,
                help,
            });
        }

        // Normal view
        // left panel
        Element agent_panel_element;
        if (agents.empty()) {
            agent_panel_element = text("No agents running") | dim | center;
        } else {
            agent_panel_element = agent_panel_container->Render();
        }

        // right panel — always refresh preview for live updates
        if (!agents.empty()) {
            int idx = std::clamp(selected_agent_index, 0, (int)agents.size() - 1);
            preview_text = agents[idx]->get_preview();
        } else {
            preview_text.clear();
        }

        Element detail_panel_element;
        if (agents.empty() || preview_text.empty()) {
            detail_panel_element = text("No agent selected") | dim | center;
        } else {
            detail_panel_element = ansi::ansi_to_element(preview_text) | flex;
        }

        // full layout
        return vbox({
            hbox({
                vbox({
                    agent_panel_element | vscroll_indicator | yframe | flex,
                }) | borderStyled(Color::Blue) | size(WIDTH, EQUAL, 40),

                vbox({
                    detail_panel_element | flex,
                }) | borderStyled(Color::Cyan) | flex,
            }) | flex,
            help,
        });
    });

    bool attach_requested = false;

    renderer = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Character('g')) {
            grid_mode = !grid_mode;
            return true;
        }
        if (event == Event::Escape && grid_mode) {
            grid_mode = false;
            return true;
        }
        if (event == Event::Character('n')) { agent_create(); return true; }
        if (event == Event::Character('s')) { agent_start(); return true; }
        if (event == Event::Character('d')) { agent_kill(); return true; }
        if (event == Event::Character('q')) { screen.Exit(); return true; }
        if (event == Event::Return) {
            if (!agents.empty() && selected_agent_index >= 0 && selected_agent_index < (int)agents.size()) {
                attach_requested = true;
                screen.Exit();
            }
            return true;
        }
        return false;
    });

    std::atomic<bool> app_running{true};
    std::thread([&] {
        while (app_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (app_running) screen.PostEvent(Event::Custom);
        }
    }).detach();

    bool running = true;
    while (running) {
        screen.Loop(renderer);
        if (attach_requested) {
            agents[selected_agent_index]->attach();
            attach_requested = false;
        } else {
            running = false;
        }
    }
    app_running = false;
  } catch (const std::exception& e) {
    LOG("(MAIN) fatal exception:", e.what());
    return 1;
  }
}
