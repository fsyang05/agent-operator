#include <memory>
#include <thread>

#include "app/app.hpp"
#include "ui/tui.hpp"
#include "util/logger.hpp"

int main()
{
    Logger::instance().set_logfile_dir("app.log");

    try {
        App app;

        TUI tui(app.tui_queue());
        app.set_tui(&tui);

        while (true) {
            std::thread tui_thread([&tui] { tui.run(); });
            app.run();
            tui_thread.join();

            if (app.last_action() == App::Action::Attach) {
                app.attach_agent(app.attach_pane_id());
                app.reset_for_reentry();
            } else {
                break;
            }
        }
    } catch (const std::exception& e) {
        LOG("(MAIN) fatal exception:", e.what());
        return 1;
    }
}
