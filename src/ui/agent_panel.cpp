#include "agent_panel.hpp"

Component AgentPanel(const Agent& agent)
{
    return Renderer([&](bool focused) {
        auto element = hbox({
                text(agent.agent_name) | bold,
                text(state_to_str(agent.state)) | bold,
                filler()
        });
        return element | (focused ? inverted : nothing);
    });
}
