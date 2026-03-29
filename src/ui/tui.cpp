#include "ui/tui.hpp"
#include "ui/ansi_parser.hpp"
#include "ui/components.hpp"

using namespace ftxui;

// --- BSP tree helpers -------------------------------------------------------

BspNode* find_node(BspNode* node, int id)
{
    if (!node) return nullptr;
    if (node->is_leaf() && node->id == id) return node;
    if (auto* r = find_node(node->left.get(), id)) return r;
    return find_node(node->right.get(), id);
}

BspNode* find_parent(BspNode* node, int target_id)
{
    if (!node || node->is_leaf()) return nullptr;
    if (BspNode::is_leaf_with_id(node->left, target_id) ||
        BspNode::is_leaf_with_id(node->right, target_id))
        return node;
    if (auto* r = find_parent(node->left.get(), target_id)) return r;
    return find_parent(node->right.get(), target_id);
}

int first_leaf_id(BspNode* node)
{
    if (!node) return -1;
    if (node->is_leaf()) return node->id;
    return first_leaf_id(node->left.get());
}

int count_leaves(BspNode* node)
{
    if (!node) return 0;
    if (node->is_leaf()) return 1;
    return count_leaves(node->left.get()) + count_leaves(node->right.get());
}

// --- TUI --------------------------------------------------------------------

TUI::TUI(EventQueue<TUIEvent>& queue)
    : event_queue_{ queue }
    , screen_{ ScreenInteractive::Fullscreen() }
{}

void TUI::run()
{
    insert_mode_ = false;
    show_quit_dialog_ = false;

    // Initialize BSP tree with a single pane if empty
    if (!bsp_root_) {
        // Create first agent on startup
        event_queue_.push(TUIEvent{ .type = TUIEventType::CreateAgent, .pane_id = 0, .keys = {} });
        bsp_root_ = BspNode::make_leaf(0);
        focused_id_ = 0;
        next_pane_id_ = 1;
    }

    root_container_ = Container::Tab({}, &tab_index_);

    rebuild_ = [this]() {
        root_container_->DetachAllChildren();
        leaf_map_.clear();
        auto app = build_app();
        root_container_->Add(app);
        if (auto it = leaf_map_.find(focused_id_); it != leaf_map_.end()) {
            it->second->TakeFocus();
        }
    };
    rebuild_();

    screen_.Loop(root_container_);
}

void TUI::update(const std::map<int, std::unique_ptr<agent::Agent>>& agents)
{
    {
        std::lock_guard lock(render_mutex_);
        agent_views_.clear();
        for (auto& [id, ag] : agents) {
            agent_views_[id] = AgentView{
                ag->name(),
                ag->agent_state(),
                ag->preview(),
            };
        }
    }
    screen_.PostEvent(Event::Custom);
}

void TUI::post_event()
{
    screen_.PostEvent(Event::Custom);
}

// --- Tree operations --------------------------------------------------------

void TUI::split_pane(SplitDir dir)
{
    BspNode* target = find_node(bsp_root_.get(), focused_id_);
    if (!target || !target->is_leaf()) return;

    int old_id = target->id;
    int new_id = next_pane_id_++;

    // Request App to create a new agent for the new pane
    event_queue_.push(TUIEvent{ .type = TUIEventType::CreateAgent, .pane_id = new_id, .keys = {} });

    auto left_child = BspNode::make_leaf(old_id);
    auto right_child = BspNode::make_leaf(new_id);

    int available;
    if (dir == SplitDir::VERTICAL) {
        available = target->last_box.x_max - target->last_box.x_min + 1;
    } else {
        available = target->last_box.y_max - target->last_box.y_min + 1;
    }
    int half = (available - 1) / 2;
    if (half < 1) half = 1;

    target->dir = dir;
    target->split_size = half;
    target->left = std::move(left_child);
    target->right = std::move(right_child);

    focused_id_ = old_id;
}

void TUI::delete_pane()
{
    if (!bsp_root_ || bsp_root_->is_leaf()) return;

    BspNode* parent = find_parent(bsp_root_.get(), focused_id_);
    if (!parent) return;

    bool focused_is_left = BspNode::is_leaf_with_id(parent->left, focused_id_);
    if (!focused_is_left && !BspNode::is_leaf_with_id(parent->right, focused_id_))
        return;

    // Tell App to delete the agent at the focused pane's index
    event_queue_.push(TUIEvent{ .type = TUIEventType::DeleteAgent, .pane_id = focused_id_, .keys = {} });

    auto sibling = focused_is_left ? std::move(parent->right)
                                   : std::move(parent->left);

    int new_focus = first_leaf_id(sibling.get());
    parent->id = sibling->id;
    parent->dir = sibling->dir;
    parent->split_size = sibling->split_size;
    parent->left = std::move(sibling->left);
    parent->right = std::move(sibling->right);

    focused_id_ = new_focus;
}

// --- Component building -----------------------------------------------------

Component TUI::build_component(BspNode* node)
{
    if (node->is_leaf()) {
        int id = node->id;
        Box* box_ptr = &node->last_box;

        auto comp = Renderer([this, id, box_ptr](bool focused) -> Element {
            if (focused) focused_id_ = id;

            std::lock_guard lock(render_mutex_);

            Element content;
            auto it = agent_views_.find(id);
            if (it != agent_views_.end()) {
                auto& av = it->second;
                Color sc = state_to_color(av.state);

                auto header = hbox({
                    text(av.name) | bold,
                    text(" "),
                    text(agent::state_to_str(av.state)) | color(sc) | bold,
                });

                auto body = ansi::ansi_to_element(av.preview) | flex;

                content = vbox({
                    header,
                    separatorLight(),
                    body,
                }) | flex;
            } else {
                content = text("Loading...") | dim | center | flex;
            }

            if (focused && insert_mode_) {
                content = content | bgcolor(Color::Palette256(22));
            } else if (focused) {
                content = content | bgcolor(Color::Palette256(235));
            }

            return content | reflect(*box_ptr);
        });

        leaf_map_[id] = comp;
        return comp;
    }

    auto left_comp = build_component(node->left.get());
    auto right_comp = build_component(node->right.get());

    if (node->dir == SplitDir::VERTICAL) {
        return ResizableSplitLeft(left_comp, right_comp, &node->split_size);
    } else {
        return ResizableSplitTop(left_comp, right_comp, &node->split_size);
    }
}

Component TUI::build_app()
{
    auto pane_tree = build_component(bsp_root_.get());

    auto with_events = CatchEvent(pane_tree, [this](Event event) -> bool {
        // Insert mode: forward everything except Escape to tmux
        if (insert_mode_) {
            if (event == Event::Escape) {
                insert_mode_ = false;
                return true;
            }
            // Let FTXUI internal events (like Custom for re-render) pass through
            if (event == Event::Custom) return false;
            forward_event_to_agent(event);
            return true;
        }

        // Normal mode keybindings
        if (event == Event::Character('n')) {
            split_pane(SplitDir::VERTICAL);
            rebuild_();
            return true;
        }
        if (event == Event::Character('N')) {
            split_pane(SplitDir::HORIZONTAL);
            rebuild_();
            return true;
        }
        if (event == Event::Character('d')) {
            delete_pane();
            rebuild_();
            return true;
        }
        if (event == Event::Character('s')) {
            event_queue_.push(TUIEvent{ .type = TUIEventType::StartAgent, .pane_id = focused_id_, .keys = {} });
            return true;
        }
        if (event == Event::Character('i')) {
            insert_mode_ = true;
            return true;
        }
        if (event == Event::Return) {
            if (focused_id_ >= 0) {
                event_queue_.push(TUIEvent{ TUIEventType::AttachAgent, focused_id_, {} });
                screen_.Exit();
            }
            return true;
        }
        if (event == Event::Character('q')) {
            show_quit_dialog_ = true;
            return true;
        }

        // Let FTXUI handle h/j/k/l navigation natively
        return false;
    });

    auto main_app = Renderer(with_events, [this, with_events] {
        int n_panes = count_leaves(bsp_root_.get());

        Element status;
        if (insert_mode_) {
            status = hbox({
                text(" -- INSERT -- ") | bold | color(Color::Green),
                filler(),
                text(" ESC") | bold | color(Color::Cyan),
                text(":normal "),
                separatorLight(),
                text(" Pane: " + std::to_string(focused_id_) + " ") | bold,
                separatorLight(),
                text(" " + std::to_string(n_panes) + " panes ") | dim,
            });
        } else {
            status = hbox({
                text(" n") | bold | color(Color::Cyan),
                text(":vsplit "),
                text("N") | bold | color(Color::Cyan),
                text(":hsplit "),
                separatorLight(),
                text(" s") | bold | color(Color::Cyan),
                text(":start "),
                text("d") | bold | color(Color::Red),
                text(":kill "),
                separatorLight(),
                text(" i") | bold | color(Color::Green),
                text(":insert "),
                text("ret") | bold | color(Color::Cyan),
                text(":attach "),
                text("q") | bold,
                text(":quit "),
                filler(),
                text(" Pane: " + std::to_string(focused_id_) + " ") | bold,
                separatorLight(),
                text(" " + std::to_string(n_panes) + " panes ") | dim,
            });
        }

        return vbox({
            with_events->Render() | flex,
            status,
        });
    });

    auto quit_dialog = CatchEvent(
        Renderer([] {
            return vbox({
                text(" Kill all agents and quit? ") | bold,
                separator(),
                hbox({
                    text(" y") | bold | color(Color::Green),
                    text(":confirm  "),
                    text("n") | bold | color(Color::Red),
                    text(":cancel "),
                }),
            }) | border | clear_under | center;
        }),
        [this](Event event) -> bool {
            if (event == Event::Character('y') || event == Event::Character('Y')) {
                event_queue_.push(TUIEvent{ TUIEventType::Quit, 0, {} });
                screen_.Exit();
                return true;
            }
            if (event == Event::Character('n') || event == Event::Character('N') ||
                event == Event::Escape) {
                show_quit_dialog_ = false;
                return true;
            }
            return true;  // swallow all events while dialog is shown
        }
    );

    return Modal(main_app, quit_dialog, &show_quit_dialog_);
}

// --- Insert mode key forwarding ---------------------------------------------

void TUI::forward_event_to_agent(const Event& event)
{
    if (focused_id_ < 0) return;

    // Special keys: send as tmux key names (not literal text)
    if (event == Event::Return) {
        event_queue_.push(TUIEvent{ TUIEventType::SendSpecialKey, focused_id_, "Enter" });
        return;
    }
    if (event == Event::Backspace) {
        event_queue_.push(TUIEvent{ TUIEventType::SendSpecialKey, focused_id_, "BSpace" });
        return;
    }
    if (event == Event::Tab) {
        event_queue_.push(TUIEvent{ TUIEventType::SendSpecialKey, focused_id_, "Tab" });
        return;
    }

    // Printable characters
    if (event.is_character()) {
        event_queue_.push(TUIEvent{ TUIEventType::SendKeys, focused_id_, event.character() });
        return;
    }

    // Other events with raw input
    auto& input = event.input();
    if (!input.empty()) {
        event_queue_.push(TUIEvent{ TUIEventType::SendKeys, focused_id_, input });
    }
}
