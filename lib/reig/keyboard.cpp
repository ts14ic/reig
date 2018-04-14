#include "keyboard.h"
#include "context.h"
#include <vector>
#include <algorithm>
#include <cctype>

namespace reig::detail {
    void Keyboard::press_key(int key) {
        mKeyCode = key;
    }

    void Keyboard::press_modifier(KeyModifier modifier) {
        if (!is_modifier_pressed(modifier)) {
            mModifiers.push_back(modifier);
        }
    }

    void Keyboard::reset() {
        mKeyCode = 0;
        mModifiers.clear();
    }

    bool Keyboard::is_modifier_pressed(reig::KeyModifier modifier) const {
        using std::begin;
        using std::end;
        return std::find(begin(mModifiers), end(mModifiers), modifier) != end(mModifiers);
    }

    Key Keyboard::get_pressed_key_type() const {
        int from = ' ';
        int to = from + 95;
        return mKeyCode == 0 ? Key::NONE :
               mKeyCode >= from && mKeyCode < to ? Key::CHAR :
               mKeyCode == '\b' ? Key::BACKSPACE :
               mKeyCode == '\033' ? Key::ESCAPE :
               Key::UNKNOWN;
    }

    int Keyboard::get_pressed_char() const {
        if (is_modifier_pressed(KeyModifier::SHIFT)) {
            return std::toupper(static_cast<unsigned char>(mKeyCode));
        } else {
            return mKeyCode;
        }
    }
}
