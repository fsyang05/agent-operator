#include "agent_panel.hpp"

Component AgentPanel(const Agent& agent)
{
    return Renderer([&](bool focused) {
        Color state_color = state_to_color(agent.state);
        auto element = hbox({
                text(agent.agent_name),
                filler(),
                text(state_to_str(agent.state)) | color(state_color) | bold,
        });
        if (focused) {
            element = element | borderStyled(state_color);
        } else {
            element = element | borderStyled(Color::GrayDark);
        }
        return element;
    });
}
