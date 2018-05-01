#ifndef REIG_WINDOW_H
#define REIG_WINDOW_H

#include "primitive.h"
#include "gsl.h"

namespace reig::detail {
    class Window {
    public:
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

        void set_title(gsl::czstring title) {
            _title = title;
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

        bool is_collapsed() const { return _is_collapsed; }

        void set_collapsed(bool is_collapsed) { _is_collapsed = is_collapsed; }

        bool is_queued() const { return _is_queued; }

        void set_queued(bool is_queued) { _is_queued = is_queued; }

    private:
        DrawData _draw_data;
        gsl::czstring _title = "";
        gsl::czstring _id = nullptr;
        float _x = 0.0f;
        float _y = 0.0f;
        float _width = 0.0f;
        float _height = 0.0f;
        float _title_bar_height = 0.0f;
        bool _is_queued = true;
        bool _is_collapsed = false;
    };

    /**
     * Increase the window's width and height to fit rect's bottom right point
     * Shift rect's y down to accommodate window's title bar
     * Reset rect's position if it's top left corner can't be fitted
     * @param rect The rectangle to accommodate
     */
    void fit_rect_in_window(primitive::Rectangle& rect, Window& window);

    primitive::Rectangle get_window_full_rect(const Window& window);

    primitive::Rectangle get_window_header_rect(const Window& window);

    primitive::Triangle get_window_minimize_triangle(const Window& window);

    primitive::Rectangle get_window_minimize_rect(const Window& window);

    primitive::Rectangle get_window_body_rect(const Window& window);

    void restart_window(Window& old_window, gsl::czstring title);
}

#endif //REIG_WINDOW_H
