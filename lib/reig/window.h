#ifndef REIG_WINDOW_H
#define REIG_WINDOW_H

#include "primitive.h"
#include "gsl.h"

namespace reig::detail {
    struct Window {
        Window(gsl::czstring title, float x, float y, float width, float height, float titleBarHeight)
                : Window{title, title, x, y, width, height, titleBarHeight} {}

        Window(gsl::czstring id, gsl::czstring title, float x, float y, float width, float height, float titleBarHeight)
                : title{title}, id{id}, x{x}, y{y}, width{width}, height{height}, title_bar_height{titleBarHeight} {}

        DrawData draw_data;
        gsl::czstring title = "";
        gsl::czstring id = nullptr;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.f;
        float height = 0.f;
        float title_bar_height = 0.f;
        bool is_finished = false;
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
