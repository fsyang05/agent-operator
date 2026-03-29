#pragma once

#include <ftxui/dom/elements.hpp>

#include "model/agent.hpp"

inline ftxui::Color state_to_color(agent::AgentState as)
{
    using enum agent::AgentState;
    switch (as) {
        case AGENT_RUNNING              :   return ftxui::Color::Green;
        case AGENT_IDLE                 :   return ftxui::Color::Yellow;
        case AGENT_PERMISSION_REQUIRED  :   return ftxui::Color::Red;
        case AGENT_STOPPED              :   return ftxui::Color::GrayDark;
    }
    return ftxui::Color::White;
}
