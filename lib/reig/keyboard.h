#ifndef REIG_KEYBOARD_H
#define REIG_KEYBOARD_H

#include "context_fwd.h"
#include <vector>

namespace reig {
    enum class Key {
        kNone,
        kChar,
        kReturn,
        kBackspace,
        kEscape
    };

    enum class KeyModifier {
        kShift
    };

    namespace detail {
        /**
         * This is pretty bad approach that assumes we are on Intel keyboard.
         * Ideally shifting characters should not be our concern
         *
         * @param ch The character that is to be transformed to upper
         * @return The shifted version of the character
         */
        int shift_to_upper(int ch);

        class Keyboard {
        public:
            Keyboard() = default;

            Keyboard(const Keyboard& other) = delete;

            Keyboard(Keyboard&& other) = delete;

            Keyboard& operator=(const Keyboard& other) = delete;

            Keyboard& operator=(Keyboard&& other) = delete;

            void press_key(int key);

            void press_special_key(Key key);

            void press_modifier(KeyModifier modifier);

            bool is_modifier_pressed(KeyModifier modifier) const;

            Key get_pressed_key_type() const;

            int get_pressed_char() const;

        private:
            friend ::reig::Context;
            void reset();

            std::vector<KeyModifier> mModifiers;
            Key mSpecialKey = Key::kNone;
            int mKeyCode = 0;
        };
    }
}

#endif //REIG_KEYBOARD_H
