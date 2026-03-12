#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "agent.hpp"
#include "tmux_session.hpp"
#include "ui/components.hpp"
#include "util/logger.hpp"

using namespace ftxui;
using namespace agent;

int main() {
  try {
    // setup logger
    Logger::instance().set_logfile_dir("app.log");

    auto screen = ScreenInteractive::Fullscreen();

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
            agents.erase(agents.begin() + idx);
            agent_panel_build_container();
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

        // full layout
        return hbox({
            vbox({
                text("Agents") | bold | center,
                separator(),
                agent_panel_element | vscroll_indicator | yframe | flex,
                separator(),
                text("n:spawn d:kill enter:attach q:quit") | dim,
            }) | border | size(WIDTH, EQUAL, 30),

            vbox({
                text("Details") | bold | center,
                separator(),
                detail_panel_element | flex,
            }) | border | flex,
        });
    });

    bool attach_requested = false;

    renderer = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Character('n')) { agent_create(); return true; }
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
