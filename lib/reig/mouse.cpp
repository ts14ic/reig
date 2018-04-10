#include "mouse.h"

using namespace reig::primitive;

void reig::detail::Mouse::move(float difx, float dify) {
    mCursorPos.x += difx;
    mCursorPos.y += dify;
}

void reig::detail::Mouse::place(float x, float y) {
    mCursorPos.x = x;
    mCursorPos.y = y;
}

void reig::detail::Mouse::scroll(float dy) {
    mScrolled = dy;
}

const Point& reig::detail::Mouse::get_cursor_pos() const {
    return mCursorPos;
}

float reig::detail::Mouse::get_scrolled() const {
    return mScrolled;
}

void reig::detail::MouseButton::press(float x, float y) {
    if (!mIsPressed) {
        mIsPressed = true;
        mIsClicked = true;
        mClickedPos = {x, y};
    }
}

void reig::detail::MouseButton::release() {
    mIsPressed = false;
}

const Point& reig::detail::MouseButton::get_clicked_pos() const {
    return mClickedPos;
}

bool reig::detail::MouseButton::is_pressed() const {
    return mIsPressed;
}

bool reig::detail::MouseButton::is_clicked() const {
    return mIsClicked;
}
