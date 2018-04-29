#ifndef REIG_WINDOW_H
#define REIG_WINDOW_H

#include "primitive.h"
#include "gsl.h"

namespace reig::detail {
    class Window {
    public:
        Window(gsl::czstring title, float x, float y, float width, float height, float title_bar_height)
                : Window{title, title, x, y, width, height, title_bar_height} {}

        Window(gsl::czstring id, gsl::czstring title, float x, float y,
               float width, float height, float title_bar_height)
                : _title{title}, _id{id}, _x{x}, _y{y}, _width{width}, _height{height},
                  _title_bar_height{title_bar_height} {}

        DrawData& draw_data() {
            return _draw_data;
        }

        const DrawData& draw_data() const {
            return _draw_data;
        }

        gsl::czstring title() const {
            return _title;
        }

        gsl::czstring id() const {
            return _id;
        }

        float x() const { return _x; }

        void set_x(float x) { _x = x; }

        float y() const { return _y; }

        void set_y(float y) { _y = y; }

        float width() const { return _width; }

        void set_width(float width) { _width = width; }

        float height() const { return _height; }

        void set_height(float height) { _height = height; }

        float title_bar_height() const { return _title_bar_height; }

        bool is_finished() const { return _is_finished; }

        void finish() { _is_finished = true; }

    private:
        DrawData _draw_data;
        gsl::czstring _title = "";
        gsl::czstring _id = nullptr;
        float _x = 0.0f;
        float _y = 0.0f;
        float _width = 0.f;
        float _height = 0.f;
        float _title_bar_height = 0.f;
        bool _is_finished = false;
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

    void update_window(Window& old_window, const Window& new_window);
}

#endif //REIG_WINDOW_H
