#include "mouse.h"
#include "maths.h"
#include "context.h"

using std::vector;
using std::reference_wrapper;
using namespace reig::primitive;

namespace reig::detail {
    Mouse::Mouse(reig::Context& context) : left_button{*this}, right_button{*this}, mContext{context} {}

    void Mouse::move(float difx, float dify) {
        mCursorPos.x += difx;
        mCursorPos.y += dify;
    }

    void Mouse::place(float x, float y) {
        mCursorPos.x = x;
        mCursorPos.y = y;
    }

    void Mouse::scroll(float dy) {
        _scrolled = dy;
    }

    const Point& Mouse::get_cursor_pos() const {
        return mCursorPos;
    }

    float Mouse::get_scrolled() const {
        return _scrolled;
    }

    bool Mouse::is_hovering_over_rect(const Rectangle& rect) const {
        bool hoveringOverRect = is_point_in_rect(mCursorPos, rect);
        if (mContext._dragged_window || !hoveringOverRect) return false;

        bool isRectVisible = mContext.if_on_top([this](Window* currentWindow, Window& previousWindow) {
            return is_point_in_rect(mCursorPos, get_full_window_rect(previousWindow));
        });

        return hoveringOverRect && isRectVisible;
    }

    MouseButton::MouseButton(Mouse& mouse) : mMouse{mouse} {}

    void MouseButton::press(float x, float y) {
        if (!mIsPressed) {
            mIsPressed = true;
            _is_clicked = true;
            _clicked_pos = {x, y};
        }
    }

    void MouseButton::release() {
        mIsPressed = false;
    }

    const Point& MouseButton::get_clicked_pos() const {
        return _clicked_pos;
    }

    bool MouseButton::is_held() const {
        return mIsPressed;
    }

    bool MouseButton::is_clicked() const {
        return _is_clicked;
    }

    bool MouseButton::clicked_in_rect(const primitive::Rectangle& rect) const {
        bool clickedInRect = is_point_in_rect(_clicked_pos, rect);
        if (mMouse.mContext._dragged_window || !clickedInRect) return false;

        bool isRectVisible = mMouse.mContext.if_on_top([this](Window* currentWindow, Window& previousWindow) {
            return is_point_in_rect(_clicked_pos, get_full_window_rect(previousWindow));
        });

        return clickedInRect && isRectVisible;
    }

    bool MouseButton::just_clicked_in_rect(const primitive::Rectangle& rect) const {
        bool justClickedInRect = _is_clicked && clicked_in_rect(rect);
        if (mMouse.mContext._dragged_window || !justClickedInRect) return false;

        bool isRectVisible = mMouse.mContext.if_on_top([this](Window* currentWindow, Window& previousWindow) {
            return clicked_in_rect(get_full_window_rect(previousWindow));
        });

        return justClickedInRect && isRectVisible;
    }

    bool MouseButton::just_clicked_in_rect_ignore_windows(const primitive::Rectangle& rect) const {
        return _is_clicked && is_point_in_rect(_clicked_pos, rect);
    }
}
