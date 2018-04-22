#ifndef REIG_WINDOW_H
#define REIG_WINDOW_H

#include "primitive.h"

namespace reig::detail {
    struct Window {
        Window(const char* title, float x, float y, float width, float height, float titleBarHeight)
                : title{title}, x{x}, y{y}, width{width}, height{height}, titleBarHeight{titleBarHeight} {}

        DrawData drawData;
        const char* title = "";
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.f;
        float height = 0.f;
        float titleBarHeight = 0.f;
        bool isFinished = false;
    };

    /**
     * Increase the window's width and height to fit rect's bottom right point
     * Shift rect's y down to accommodate window's title bar
     * Reset rect's position if it's top left corner can't be fitted
     * @param rect The rectangle to accommodate
     */
    void fit_rect_in_window(primitive::Rectangle& rect, Window& window);

    primitive::Rectangle get_full_window_rect(const Window& window);

    primitive::Rectangle get_window_header_rect(const Window& window);

    primitive::Rectangle get_window_body_rect(const Window& window);
}

#endif //REIG_WINDOW_H
