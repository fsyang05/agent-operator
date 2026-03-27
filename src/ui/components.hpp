#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

#include "model/agent.hpp"

using namespace ftxui;
using enum agent::AgentState;

inline Color state_to_color(const agent::AgentState as)
{
    switch (as) {
        case AGENT_RUNNING              :   return ftxui::Color::Green;
        case AGENT_IDLE                 :   return ftxui::Color::Yellow;
        case AGENT_PERMISSION_REQUIRED  :   return ftxui::Color::Red;
        case AGENT_STOPPED              :   return ftxui::Color::GrayDark;
    }
    return ftxui::Color::White;
}

inline Component AgentPanel(const agent::Agent& agent)
{
    return Renderer([&](bool focused) {
        Color state_color = state_to_color(agent.agent_state());
        auto element = hbox({
                text(agent.name()),
                filler(),
                text(state_to_str(agent.agent_state())) | color(state_color) | bold,
                });
        if (focused) {
            element = element | borderStyled(state_color);
        } else {
            element = element | borderStyled(Color::GrayDark);
        }
        return element;
    });
}

inline Component AgentList(std::vector<agent::Agent>& agents)
{
    auto list = Container::Vertical({});
    for (auto& a : agents) {
        list->Add(AgentPanel(a));
    }
    return list;
}

inline Component TerminalPreview(const std::string& term_output)
{
    return Renderer([&] {
        return paragraph(term_output) | border | flex;
    });
}
