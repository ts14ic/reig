#ifndef REIG_MOUSE_H
#define REIG_MOUSE_H

#include "primitive.h"
#include "context_fwd.h"

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

        bool just_clicked_in_window(const primitive::Rectangle& rect) const;

        const primitive::Point& get_clicked_pos() const;

        bool is_clicked() const;

        Mouse& _mouse;
        primitive::Point _clicked_pos;
        bool _is_pressed = false;
        bool _is_clicked = false;
    };

    class Mouse {
    public:
        explicit Mouse(reig::Context& context);

        Mouse(const Mouse&) = delete;

        Mouse(Mouse&&) = delete;

        Mouse& operator=(const Mouse&) = delete;

        Mouse& operator=(Mouse&&) = delete;

        detail::MouseButton left_button;
        detail::MouseButton right_button;

        /**
         * @brief Moves cursor against previous position
         * @param on_x Delta x coordinate
         * @param on_y Delta y coordinate
         */
        void move(float on_x, float on_y);

        /**
         * @brief Places the cursors in abosulute coordinates
         * @param x X coordinate
         * @param y Y coordiante
         */
        void place(float x, float y);

        /**
         * @brief Scrolls the virtual mouse wheel
         * @param amount Amount of scrolling
         */
        void scroll(float amount);

        bool is_hovering_over_rect(const primitive::Rectangle& rect) const;

        const primitive::Point& get_cursor_pos() const;

        float get_scrolled() const;

    private:
        friend class ::reig::Context;
        friend class ::reig::detail::MouseButton;

        Context& _context;
        primitive::Point _cursor_pos;
        float _scrolled = 0.0f;
    };
}

#endif //REIG_MOUSE_H
