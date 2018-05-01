#include "window.h"

namespace reig::detail {
    void fit_rect_in_window(reig::primitive::Rectangle& rect, Window& window) {
        rect.x += window.x() + 4;
        rect.y += window.y() + window.title_bar_height() + 4;

        if (window.x() + window.width() < get_x2(rect)) {
            window.set_width(get_x2(rect) - window.x() + 4);
        }
        if (!window.is_collapsed() && window.y() + window.height() < get_y2(rect)) {
            window.set_height(get_y2(rect) - window.y() + 4);
        }
        if (rect.x < window.x()) {
            rect.x = window.x() + 4;
        }
        if (rect.y < window.y()) {
            rect.y = window.y() + 4;
        }
    }

    primitive::Rectangle get_window_full_rect(const Window& window) {
        return {window.x(), window.y(), window.width(), window.height()};
    }

    primitive::Rectangle get_window_header_rect(const Window& window) {
        return {window.x(), window.y(), window.width(), window.title_bar_height()};
    }

    primitive::Triangle get_window_minimize_triangle(const Window& window) {
        return {{window.x() + 3.f,                                   window.y() + 3.f},
                {window.x() + 3.f + window.title_bar_height(),       window.y() + 3.f},
                {window.x() + 3.f + window.title_bar_height() / 2.f, window.y() + window.title_bar_height() - 3.f}};
    }

    primitive::Rectangle get_window_minimize_rect(const Window& window) {
        auto triangle = get_window_minimize_triangle(window);
        return {triangle.pos0.x, triangle.pos0.y, triangle.pos1.x - triangle.pos0.x, triangle.pos2.y - triangle.pos0.y};
    }

    primitive::Rectangle get_window_body_rect(const Window& window) {
        return {window.x(), window.y() + window.title_bar_height(), window.width(), window.height() - window.title_bar_height()};
    }

    void restart_window(Window& window, gsl::czstring title) {
        window.set_queued(true);
        window.set_finished(false);
        window.set_title(title);
        window.set_width(0.0f);
        window.set_height(window.title_bar_height());
    }
}