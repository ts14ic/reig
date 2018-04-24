#include "mouse.h"
#include "maths.h"
#include "context.h"

using std::vector;
using std::reference_wrapper;
using namespace reig::primitive;

namespace reig::detail {
    Mouse::Mouse(reig::Context& context) : left_button{*this}, right_button{*this}, _context{context} {}

    void Mouse::move(float on_x, float on_y) {
        _cursor_pos.x += on_x;
        _cursor_pos.y += on_y;
    }

    void Mouse::place(float x, float y) {
        _cursor_pos.x = x;
        _cursor_pos.y = y;
    }

    void Mouse::scroll(float amount) {
        _scrolled = amount;
    }

    const Point& Mouse::get_cursor_pos() const {
        return _cursor_pos;
    }

    float Mouse::get_scrolled() const {
        return _scrolled;
    }

    bool Mouse::is_hovering_over_rect(const Rectangle& rect) const {
        bool hovering_over_rect = is_point_in_rect(_cursor_pos, rect);
        if (_context._dragged_window || !hovering_over_rect) return false;

        bool is_rect_visible = _context.if_on_top([this](Window* current_window, Window& previous_window) {
            return is_point_in_rect(_cursor_pos, get_full_window_rect(previous_window));
        });

        return hovering_over_rect && is_rect_visible;
    }

    MouseButton::MouseButton(Mouse& mouse) : _mouse{mouse} {}

    void MouseButton::press(float x, float y) {
        if (!_is_pressed) {
            _is_pressed = true;
            _is_clicked = true;
            _clicked_pos = {x, y};
        }
    }

    void MouseButton::release() {
        _is_pressed = false;
    }

    const Point& MouseButton::get_clicked_pos() const {
        return _clicked_pos;
    }

    bool MouseButton::is_held() const {
        return _is_pressed;
    }

    bool MouseButton::is_clicked() const {
        return _is_clicked;
    }

    bool MouseButton::clicked_in_rect(const primitive::Rectangle& rect) const {
        bool clicked_in_rect = is_point_in_rect(_clicked_pos, rect);
        if (_mouse._context._dragged_window || !clicked_in_rect) return false;

        bool is_rect_visible = _mouse._context.if_on_top([this](Window* current_window, Window& previous_window) {
            return is_point_in_rect(_clicked_pos, get_full_window_rect(previous_window));
        });

        return clicked_in_rect && is_rect_visible;
    }

    bool MouseButton::just_clicked_in_rect(const primitive::Rectangle& rect) const {
        bool just_clicked_in_rect = _is_clicked && clicked_in_rect(rect);
        if (_mouse._context._dragged_window || !just_clicked_in_rect) return false;

        bool is_rect_visible = _mouse._context.if_on_top([this](Window* current_window, Window& previous_window) {
            return clicked_in_rect(get_full_window_rect(previous_window));
        });

        return just_clicked_in_rect && is_rect_visible;
    }

    bool MouseButton::just_clicked_in_rect_ignore_windows(const primitive::Rectangle& rect) const {
        return _is_clicked && is_point_in_rect(_clicked_pos, rect);
    }
}
