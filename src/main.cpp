#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "agent.hpp"

using namespace ftxui;

int main() {
    auto screen = ScreenInteractive::Fullscreen();
    int counter = 0;

    std::vector<Agent> agents;

    // left panel
    int agent_panel_focus_index = 0;
    auto agent_panel_container = Container::Vertical({}, &agent_panel_focus_index);

    // refresh the component when data changes
    // TODO
    auto agent_panel_rebuild_container = [&] {
        agent_panel_container->DetachAllChildren();
        for (int i = 0; i < (int)agents.size(); i++) {

            // note: i must be captured by value, not reference
            // since this loop is run inside Render() later
            auto agent_panel = Renderer([&agents, i](bool focused) {
                auto style = focused ? inverted : nothing;
                return hbox({
                    text("") | bold,
                    filler(),
                }) | style;
            });

            agent_panel_container->Add(agent_panel);
        }
    };

    auto agent_create = [&] {
        // TODO
        agent_panel_rebuild_container();
    };

    auto agent_kill = [&] {
        // TODO
        agent_panel_rebuild_container();
    };

    auto renderer = Renderer(agent_panel_container, [&] {
        // left panel
        // TODO
        Element agent_panel_element = vbox({
            text("No agents running") | dim | center,
        });

        // right panel
        // TODO
        Element detail_panel_element = vbox({
            text("No agent selected") | dim | center,
        });

        // full layout
        return hbox ({
            vbox({
                text("Agents") | bold | center,
                separator(),
                agent_panel_element | vscroll_indicator | yframe | flex,
                separator(),
                text("n:spawn d:kill q:quit") | dim,
            }) | border | size(WIDTH, EQUAL, 30),

            vbox({
                text("Details") | bold | center,
                separator(),
                detail_panel_element | flex,
            }) | border | flex,
        });
    });

    renderer = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Character('n')) { agent_create(); return true; }
        if (event == Event::Character('d')) { agent_kill(); return true; }
        if (event == Event::Character('q')) { screen.Exit(); return true; }
        return false;
    });

    screen.Loop(renderer);
}
