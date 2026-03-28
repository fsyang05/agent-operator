#include "ui/tui.hpp"
#include "ui/ansi_parser.hpp"
#include "ui/components.hpp"

using namespace ftxui;

TUI::TUI(EventQueue<TUIEvent>& queue)
    : event_queue_{ queue }
    , screen_{ ScreenInteractive::Fullscreen() }
{}

void TUI::run()
{
    screen_.Loop(build_ui());
}

void TUI::update(const std::vector<std::unique_ptr<agent::Agent>>& agents)
{
    {
        std::lock_guard lock(render_mutex_);
        agent_views_.resize(agents.size());
        for (size_t i = 0; i < agents.size(); ++i) {
            agent_views_[i].name = agents[i]->name();
            agent_views_[i].state = agents[i]->agent_state();
            agent_views_[i].preview = agents[i]->preview();
        }
    }
    screen_.PostEvent(Event::Custom);
}

void TUI::post_event()
{
    screen_.PostEvent(Event::Custom);
}

Element TUI::render_help_bar()
{
    return hbox({
        text("n") | bold | color(Color::BlueLight), text(":spawn ") | dim,
        text("s") | bold | color(Color::BlueLight), text(":start ") | dim,
        text("d") | bold | color(Color::BlueLight), text(":kill ") | dim,
        text("ret") | bold | color(Color::BlueLight), text(":attach ") | dim,
        text("g") | bold | color(Color::BlueLight), text(":grid ") | dim,
        text("q") | bold | color(Color::BlueLight), text(":quit") | dim,
    });
}

Element TUI::render_grid_view()
{
    constexpr int cols = 3;
    int n = static_cast<int>(agent_views_.size());

    if (n == 0)
        return text("No agents") | dim | center | flex;

    std::vector<Elements> grid_rows;
    Elements current_row;

    for (int i = 0; i < n; ++i) {
        auto& av = agent_views_[i];
        Color sc = state_to_color(av.state);

        auto cell = vbox({
            hbox({
                text(av.name) | bold,
                text(" "),
                text(agent::state_to_str(av.state)) | color(sc) | bold,
            }),
            separator(),
            ansi::ansi_to_element(av.preview) | flex,
        }) | borderStyled(sc) | flex;

        current_row.push_back(std::move(cell));
        if (static_cast<int>(current_row.size()) == cols) {
            grid_rows.push_back(std::move(current_row));
            current_row.clear();
        }
    }

    while (static_cast<int>(current_row.size()) < cols)
        current_row.push_back(vbox({}) | flex);

    if (!current_row.empty())
        grid_rows.push_back(std::move(current_row));

    return gridbox(std::move(grid_rows)) | flex;
}

Element TUI::render_normal_view()
{
    int n = static_cast<int>(agent_views_.size());

    // Left panel — agent list
    Element agent_panel_element;
    if (n == 0) {
        agent_panel_element = text("No agents running") | dim | center;
    } else {
        Elements items;
        for (int i = 0; i < n; ++i) {
            auto& av = agent_views_[i];
            Color sc = state_to_color(av.state);
            auto row = hbox({
                text(av.name),
                filler(),
                text(agent::state_to_str(av.state)) | color(sc) | bold,
            });
            if (i == selected_index_) {
                row = row | borderStyled(sc);
            } else {
                row = row | borderStyled(Color::GrayDark);
            }
            items.push_back(std::move(row));
        }
        agent_panel_element = vbox(std::move(items));
    }

    // Right panel — preview
    Element detail_panel_element;
    if (n == 0) {
        detail_panel_element = text("No agent selected") | dim | center;
    } else {
        int idx = std::clamp(selected_index_, 0, n - 1);
        const auto& preview = agent_views_[idx].preview;
        if (preview.empty()) {
            detail_panel_element = text("No agent selected") | dim | center;
        } else {
            detail_panel_element = ansi::ansi_to_element(preview) | flex;
        }
    }

    return hbox({
        vbox({
            agent_panel_element | vscroll_indicator | yframe | flex,
        }) | borderStyled(Color::Blue) | size(WIDTH, EQUAL, 40),
        vbox({
            detail_panel_element | flex,
        }) | borderStyled(Color::Cyan) | flex,
    }) | flex;
}

Component TUI::build_ui()
{
    auto renderer = Renderer([this] {
        std::lock_guard lock(render_mutex_);

        Element content;
        if (grid_mode_) {
            content = render_grid_view();
        } else {
            content = render_normal_view();
        }

        return vbox({
            content | flex,
            render_help_bar(),
        });
    });

    return CatchEvent(renderer, [this](Event event) {
        int n;
        {
            std::lock_guard lock(render_mutex_);
            n = static_cast<int>(agent_views_.size());
        }

        if (event == Event::Character('g')) {
            grid_mode_ = !grid_mode_;
            return true;
        }
        if (event == Event::Escape && grid_mode_) {
            grid_mode_ = false;
            return true;
        }
        if (event == Event::Character('n')) {
            event_queue_.push(TUIEvent{ TUIEventType::CreateAgent, selected_index_ });
            return true;
        }
        if (event == Event::Character('s')) {
            event_queue_.push(TUIEvent{ TUIEventType::StartAgent, selected_index_ });
            return true;
        }
        if (event == Event::Character('d')) {
            event_queue_.push(TUIEvent{ TUIEventType::DeleteAgent, selected_index_ });
            return true;
        }
        if (event == Event::Character('q')) {
            event_queue_.push(TUIEvent{ TUIEventType::Quit, selected_index_ });
            screen_.Exit();
            return true;
        }
        if (event == Event::Return) {
            if (n > 0 && selected_index_ >= 0 && selected_index_ < n) {
                event_queue_.push(TUIEvent{ TUIEventType::AttachAgent, selected_index_ });
                screen_.Exit();
            }
            return true;
        }
        // Arrow keys for selection
        if (event == Event::ArrowUp || event == Event::Character('k')) {
            if (selected_index_ > 0) selected_index_--;
            return true;
        }
        if (event == Event::ArrowDown || event == Event::Character('j')) {
            if (selected_index_ < n - 1) selected_index_++;
            return true;
        }
        return false;
    });
}
