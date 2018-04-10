#ifndef REIG_MOUSE_H
#define REIG_MOUSE_H

#include "primitive.h"
#include "fwd.h"

namespace reig::detail {
    class MouseButton {
    public:
        MouseButton() = default;

        MouseButton(MouseButton const&) = delete;

        MouseButton(MouseButton&&) = delete;

        MouseButton& operator=(MouseButton const&) = delete;

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

        const primitive::Point& get_clicked_pos() const;

        bool is_pressed() const;

        bool is_clicked() const;

    private:
        friend class ::reig::Context;

        primitive::Point mClickedPos;
        bool mIsPressed = false;
        bool mIsClicked = false;
    };

    class Mouse {
    public:
        Mouse() = default;

        Mouse(Mouse const&) = delete;

        Mouse(Mouse&&) = delete;

        Mouse& operator=(Mouse const&) = delete;

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

        const primitive::Point& get_cursor_pos() const;

        float get_scrolled() const;

    private:
        friend class ::reig::Context;

        primitive::Point mCursorPos;
        float mScrolled = 0.0f;
    };
}

#endif //REIG_MOUSE_H
