#include "window.h"

namespace reig::detail {
    void fit_rect_in_window(reig::primitive::Rectangle& rect, Window& window) {
        rect.x += window.x() + 4;
        rect.y += window.y() + window.title_bar_height() + 4;

        if (window.x() + window.width() < get_x2(rect)) {
            window.set_width(get_x2(rect) - window.x());
        }
        if (window.y() + window.height() < get_y2(rect)) {
            window.set_height(get_y2(rect) - window.y());
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
}