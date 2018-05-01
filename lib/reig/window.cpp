#include "window.h"

namespace reig::detail {
    void fit_rect_in_window(reig::primitive::Rectangle& rect, Window& window) {
        rect.x += window.x() + 4;
        rect.y += window.y() + window.title_bar_height() + 4;

        if (window.x() + window.width() < get_x2(rect)) {
            window.set_width(get_x2(rect) - window.x() + 4);
        }
        if (window.y() + window.height() < get_y2(rect)) {
            window.set_height(get_y2(rect) - window.y() + 4);
        }
        if (rect.x < window.x()) {
            rect.x = window.x() + 4;
        }
        if (rect.y < window.y()) {
            rect.y = window.y() + 4;
        }
    }

    primitive::Rectangle get_full_window_rect(const Window& window) {
        return {window.x(), window.y(), window.width(), window.height()};
    }

    primitive::Rectangle get_window_header_rect(const Window& window) {
        return {window.x(), window.y(), window.width(), window.title_bar_height()};
    }

    primitive::Rectangle get_window_body_rect(const Window& window) {
        return {window.x(), window.y() + window.title_bar_height(), window.width(), window.height() - window.title_bar_height()};
    }

    void update_window(Window& old_window, const Window& new_window) {
        old_window.set_x(new_window.x());
        old_window.set_y(new_window.y());
        old_window.set_width(new_window.width());
        old_window.set_height(new_window.height());
    }
}