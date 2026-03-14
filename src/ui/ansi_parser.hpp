#pragma once

#include <string>
#include <vector>
#include <sstream>

#include <ftxui/dom/elements.hpp>

namespace ansi {

inline ftxui::Element ansi_to_element(const std::string& input) {
    using namespace ftxui;

    // Current style state
    bool is_bold = false;
    bool is_dim = false;
    bool is_underlined = false;
    Color fg = Color::Default;
    Color bg = Color::Default;

    // Standard color mapping (30-37 / 40-47)
    auto standard_color = [](int code) -> Color {
        switch (code) {
            case 0: return Color::Black;
            case 1: return Color::Red;
            case 2: return Color::Green;
            case 3: return Color::Yellow;
            case 4: return Color::Blue;
            case 5: return Color::Magenta;
            case 6: return Color::Cyan;
            case 7: return Color::White;
            default: return Color::Default;
        }
    };

    // Bright color mapping (90-97)
    auto bright_color = [](int code) -> Color {
        switch (code) {
            case 0: return Color::GrayDark;
            case 1: return Color::RedLight;
            case 2: return Color::GreenLight;
            case 3: return Color::YellowLight;
            case 4: return Color::BlueLight;
            case 5: return Color::MagentaLight;
            case 6: return Color::CyanLight;
            case 7: return Color::GrayLight;
            default: return Color::Default;
        }
    };

    auto apply_style = [&](Element e) -> Element {
        if (fg != Color::Default) e = e | color(fg);
        if (bg != Color::Default) e = e | bgcolor(bg);
        if (is_bold) e = e | bold;
        if (is_dim) e = e | dim;
        if (is_underlined) e = e | underlined;
        return e;
    };

    // Split into lines
    std::vector<std::string> lines;
    std::istringstream stream(input);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    if (lines.empty()) return text("");

    Elements vlines;

    for (const auto& ln : lines) {
        Elements segments;
        std::string current_text;
        size_t i = 0;

        while (i < ln.size()) {
            // Check for ESC [ ... m sequence
            if (ln[i] == '\033' && i + 1 < ln.size() && ln[i + 1] == '[') {
                // Flush accumulated text
                if (!current_text.empty()) {
                    segments.push_back(apply_style(text(current_text)));
                    current_text.clear();
                }

                // Parse CSI sequence
                i += 2; // skip ESC [
                std::vector<int> params;
                std::string num;

                while (i < ln.size()) {
                    char c = ln[i];
                    if (c >= '0' && c <= '9') {
                        num += c;
                        i++;
                    } else if (c == ';') {
                        params.push_back(num.empty() ? 0 : std::stoi(num));
                        num.clear();
                        i++;
                    } else {
                        // End of sequence
                        params.push_back(num.empty() ? 0 : std::stoi(num));
                        num.clear();
                        if (c == 'm') {
                            // Process SGR params
                            for (size_t p = 0; p < params.size(); p++) {
                                int code = params[p];
                                if (code == 0) {
                                    // Reset
                                    is_bold = false;
                                    is_dim = false;
                                    is_underlined = false;
                                    fg = Color::Default;
                                    bg = Color::Default;
                                } else if (code == 1) {
                                    is_bold = true;
                                } else if (code == 2) {
                                    is_dim = true;
                                } else if (code == 4) {
                                    is_underlined = true;
                                } else if (code == 22) {
                                    is_bold = false;
                                    is_dim = false;
                                } else if (code == 24) {
                                    is_underlined = false;
                                } else if (code == 39) {
                                    fg = Color::Default;
                                } else if (code == 49) {
                                    bg = Color::Default;
                                } else if (code >= 30 && code <= 37) {
                                    fg = standard_color(code - 30);
                                } else if (code >= 40 && code <= 47) {
                                    bg = standard_color(code - 40);
                                } else if (code >= 90 && code <= 97) {
                                    fg = bright_color(code - 90);
                                } else if (code == 38 && p + 1 < params.size()) {
                                    // Extended foreground
                                    if (params[p + 1] == 5 && p + 2 < params.size()) {
                                        fg = Color::Palette256(params[p + 2]);
                                        p += 2;
                                    } else if (params[p + 1] == 2 && p + 4 < params.size()) {
                                        fg = Color::RGB(params[p + 2], params[p + 3], params[p + 4]);
                                        p += 4;
                                    }
                                } else if (code == 48 && p + 1 < params.size()) {
                                    // Extended background
                                    if (params[p + 1] == 5 && p + 2 < params.size()) {
                                        bg = Color::Palette256(params[p + 2]);
                                        p += 2;
                                    } else if (params[p + 1] == 2 && p + 4 < params.size()) {
                                        bg = Color::RGB(params[p + 2], params[p + 3], params[p + 4]);
                                        p += 4;
                                    }
                                }
                            }
                        }
                        // Skip any unrecognized final byte
                        i++;
                        break;
                    }
                }
            } else {
                current_text += ln[i];
                i++;
            }
        }

        // Flush remaining text
        if (!current_text.empty()) {
            segments.push_back(apply_style(text(current_text)));
        }

        if (segments.empty()) {
            vlines.push_back(text(""));
        } else {
            vlines.push_back(hbox(std::move(segments)));
        }
    }

    return vbox(std::move(vlines));
}

} // namespace ansi
