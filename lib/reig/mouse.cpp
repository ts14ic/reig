#include "mouse.h"
#include "internal.h"
#include "context.h"

using std::vector;
using std::reference_wrapper;
using namespace reig::primitive;

namespace reig::detail {
    Mouse::Mouse(reig::Context& context) : leftButton{*this}, rightButton{*this}, mContext{context} {}

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
        bool hoveringOverRect = internal::is_boxed_in(mCursorPos, rect);
        if (mContext.mDraggedWindow || !hoveringOverRect) return false;

        auto* currentWindow = mContext.get_current_window();
        auto& mPreviousWindows = mContext.mPreviousWindows;
        bool isRectVisible = true;
        for (auto& previousWindow : mPreviousWindows) {
            if (internal::is_boxed_in(mCursorPos, as_rect(previousWindow))) {
                if (currentWindow && !currentWindow->isFinished) {
                    return currentWindow->title == previousWindow.title;
                } else {
                    return false;
                }
            }
        }

        return hoveringOverRect && isRectVisible;
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
        bool clickedInRect = internal::is_boxed_in(mClickedPos, rect);
        if (mMouse.mContext.mDraggedWindow || !clickedInRect) return false;

        auto* currentWindow = mMouse.mContext.get_current_window();
        auto& mPreviousWindows = mMouse.mContext.mPreviousWindows;
        bool isRectVisible = true;
        for (auto& previousWindow : mPreviousWindows) {
            if (internal::is_boxed_in(mClickedPos, as_rect(previousWindow))) {
                if (currentWindow && !currentWindow->isFinished) {
                    return currentWindow->title == previousWindow.title;
                } else {
                    return false;
                }
            }
        }

        return clickedInRect && isRectVisible;
    }

    bool MouseButton::just_clicked_in_rect(const primitive::Rectangle& rect) const {
        bool justClickedInRect = mIsClicked && clicked_in_rect(rect);
        if (mMouse.mContext.mDraggedWindow || !justClickedInRect) return false;

        auto* currentWindow = mMouse.mContext.get_current_window();
        auto& mPreviousWindows = mMouse.mContext.mPreviousWindows;
        bool isRectVisible = true;
        for (auto& previousWindow : mPreviousWindows) {
            if (clicked_in_rect(as_rect(previousWindow))) {
                if (currentWindow && !currentWindow->isFinished) {
                    return currentWindow->title == previousWindow.title;
                } else {
                    return false;
                }
            }
        }

        return justClickedInRect && isRectVisible;
    }
}
