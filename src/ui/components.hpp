#pragma once

#include "agent_panel.hpp"
#include <vector>
#include <string>

inline Component AgentList(std::vector<Agent>& agents)
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
