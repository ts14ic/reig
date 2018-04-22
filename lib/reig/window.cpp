#include "window.h"

namespace reig::detail {
    void fit_rect_in_window(reig::primitive::Rectangle& rect, Window& window) {
        rect.x += window.x + 4;
        rect.y += window.y + window.titleBarHeight + 4;

        if (window.x + window.width < get_x2(rect)) {
            window.width = get_x2(rect) - window.x;
        }
        if (window.y + window.height < get_y2(rect)) {
            window.height = get_y2(rect) - window.y;
        }
        if (rect.x < window.x) {
            rect.x = window.x + 4;
        }
        if (rect.y < window.y) {
            rect.y = window.y + 4;
        }
    }

    primitive::Rectangle as_rect(const Window& window) {
        return {window.x, window.y, window.width, window.height};
    }
}