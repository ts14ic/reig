#include "mouse.h"
#include "internal.h"

namespace reig::detail {
    using namespace reig::primitive;

    Mouse::Mouse(reig::Context& context) : mContext{context}, leftButton{*this}, rightButton{*this} {}

    void Mouse::move(float difx, float dify) {
        mCursorPos.x += difx;
        mCursorPos.y += dify;
    }

    void Mouse::place(float x, float y) {
        mCursorPos.x = x;
        mCursorPos.y = y;
    }

    void Mouse::scroll(float dy) {
        mScrolled = dy;
    }

    const Point& Mouse::get_cursor_pos() const {
        return mCursorPos;
    }

    float Mouse::get_scrolled() const {
        return mScrolled;
    }

    bool Mouse::is_hovering_over_rect(const Rectangle& rect) const {
        return internal::is_boxed_in(mCursorPos, rect);
    }

    MouseButton::MouseButton(Mouse& mouse) : mMouse{mouse} {}

    void MouseButton::press(float x, float y) {
        if (!mIsPressed) {
            mIsPressed = true;
            mIsClicked = true;
            mClickedPos = {x, y};
        }
    }

    void MouseButton::release() {
        mIsPressed = false;
    }

    const Point& MouseButton::get_clicked_pos() const {
        return mClickedPos;
    }

    bool MouseButton::is_held() const {
        return mIsPressed;
    }

    bool MouseButton::is_clicked() const {
        return mIsClicked;
    }

    bool MouseButton::clicked_in_rect(const primitive::Rectangle& rect) const {
        return internal::is_boxed_in(mClickedPos, rect);
    }

    bool MouseButton::just_clicked_in_rect(const primitive::Rectangle& rect) const {
        return mIsClicked && clicked_in_rect(rect);
    }
}
