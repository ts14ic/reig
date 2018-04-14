#ifndef REIG_KEYBOARD_H
#define REIG_KEYBOARD_H

#include <vector>

namespace reig {
    enum class Key {
        NONE,
        CHAR,
        BACKSPACE,
        ESCAPE,
        UNKNOWN,
    };

    enum class KeyModifier {
        SHIFT
    };

    namespace detail {
        class Keyboard {
        public:
            Keyboard() = default;

            Keyboard(const Keyboard& other) = delete;

            Keyboard(Keyboard&& other) = delete;

            Keyboard& operator=(const Keyboard& other) = delete;

            Keyboard& operator=(Keyboard&& other) = delete;

            void press_key(int key);

            void press_modifier(KeyModifier modifier);

            void reset();

            bool is_modifier_pressed(KeyModifier modifier) const;

            Key get_pressed_key_type() const;

            int get_pressed_char() const;

        private:
            std::vector<KeyModifier> mModifiers;
            int mKeyCode = 0;
        };
    }
}

#endif //REIG_KEYBOARD_H
