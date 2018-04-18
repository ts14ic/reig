#include "mouse.h"


namespace reig::detail {
    using namespace reig::primitive;

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

    bool MouseButton::is_pressed() const {
        return mIsPressed;
    }

    bool MouseButton::is_clicked() const {
        return mIsClicked;
    }
}
