#ifndef REIG_MOUSE_H
#define REIG_MOUSE_H

#include "primitive.h"
#include "fwd.h"

namespace reig::detail {
    class Mouse;

    class MouseButton {
    public:
        explicit MouseButton(Mouse& mouse);

        MouseButton(const MouseButton&) = delete;

        MouseButton(MouseButton&&) = delete;

        MouseButton& operator=(const MouseButton&) = delete;

        MouseButton& operator=(MouseButton&&) = delete;

        /**
         * @brief Sets mouse pressed and clicked states
         * @param x X coordinate
         * @param y Y coordinate
         */
        void press(float x, float y);

        /**
         * @brief Unsets mouse pressed state
         */
        void release();

        bool clicked_in_rect(const primitive::Rectangle& rect) const;

        bool just_clicked_in_rect(const primitive::Rectangle& rect) const;

        bool is_held() const;
    private:
        friend class ::reig::Context;
        friend class ::reig::detail::Mouse;

        bool just_clicked_in_rect_ignore_windows(const primitive::Rectangle& rect) const;

        const primitive::Point& get_clicked_pos() const;

        bool is_clicked() const;

        Mouse& mMouse;
        primitive::Point mClickedPos;
        bool mIsPressed = false;
        bool mIsClicked = false;
    };

    class Mouse {
    public:
        explicit Mouse(reig::Context& context);

        Mouse(const Mouse&) = delete;

        Mouse(Mouse&&) = delete;

        Mouse& operator=(const Mouse&) = delete;

        Mouse& operator=(Mouse&&) = delete;

        detail::MouseButton leftButton;
        detail::MouseButton rightButton;

        /**
         * @brief Moves cursor against previous position
         * @param difx Delta x coordinate
         * @param dify Delta y coordinate
         */
        void move(float difx, float dify);

        /**
         * @brief Places the cursors in abosulute coordinates
         * @param x X coordinate
         * @param y Y coordiante
         */
        void place(float x, float y);

        /**
         * @brief Scrolls the virtual mouse wheel
         * @param dy Amount of scrolling
         */
        void scroll(float dy);

        bool is_hovering_over_rect(const primitive::Rectangle& rect) const;

        const primitive::Point& get_cursor_pos() const;

        float get_scrolled() const;

    private:
        friend class ::reig::Context;
        friend class ::reig::detail::MouseButton;

        Context& mContext;
        primitive::Point mCursorPos;
        float mScrolled = 0.0f;
    };
}

#endif //REIG_MOUSE_H
