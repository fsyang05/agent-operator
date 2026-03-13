#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "model/agent.hpp"
#include "model/tmux_session.hpp"
#include "router.cpp"
#include "server/http_tcp_server.hpp"
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
    int prev_selected_index = -1;
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

    auto renderer = Renderer(agent_panel_container, [&] {
        // left panel
        Element agent_panel_element;
        if (agents.empty()) {
            agent_panel_element = text("No agents running") | dim | center;
        } else {
            agent_panel_element = agent_panel_container->Render();
        }

        // right panel — rebuild preview when selection changes
        if (!agents.empty() && selected_agent_index != prev_selected_index) {
            prev_selected_index = selected_agent_index;
            int idx = std::clamp(selected_agent_index, 0, (int)agents.size() - 1);
            preview_text = agents[idx]->get_preview();
        }
        if (agents.empty()) {
            prev_selected_index = -1;
            preview_text.clear();
        }

        Element detail_panel_element;
        if (agents.empty() || preview_text.empty()) {
            detail_panel_element = text("No agent selected") | dim | center;
        } else {
            detail_panel_element = paragraph(preview_text) | flex;
        }

        // help text with highlighted keys
        auto help = hbox({
            text("n") | bold | color(Color::Magenta), text(":spawn ") | dim,
            text("s") | bold | color(Color::Magenta), text(":start ") | dim,
            text("d") | bold | color(Color::Magenta), text(":kill ") | dim,
            text("ret") | bold | color(Color::Magenta), text(":attach ") | dim,
            text("q") | bold | color(Color::Magenta), text(":quit") | dim,
        });

        // full layout
        return hbox({
            vbox({
                text("Agents") | bold | color(Color::Blue) | center,
                separator(),
                agent_panel_element | vscroll_indicator | yframe | flex,
                separator(),
                help,
            }) | borderStyled(Color::Blue) | size(WIDTH, EQUAL, 40),

            vbox({
                text("Details") | bold | color(Color::Cyan) | center,
                separator(),
                detail_panel_element | flex,
            }) | borderStyled(Color::Cyan) | flex,
        });
    });

    bool attach_requested = false;

    renderer = CatchEvent(renderer, [&](Event event) {
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
  } catch (const std::exception& e) {
    LOG("(MAIN) fatal exception:", e.what());
    return 1;
  }
}
