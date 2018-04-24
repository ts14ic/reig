#include "keyboard.h"
#include "context.h"
#include <vector>
#include <algorithm>

namespace reig::detail {
    void Keyboard::press_key(int key) {
        mKeyCode = key;
        mSpecialKey = Key::kNone;
    }

    void Keyboard::press_special_key(Key key) {
        if (key != Key::kChar) {
            mKeyCode = 0;
            mSpecialKey = key;
        }
    }

    void Keyboard::press_modifier(KeyModifier modifier) {
        if (!is_modifier_pressed(modifier)) {
            mModifiers.push_back(modifier);
        }
    }

    void Keyboard::reset() {
        mKeyCode = 0;
        mSpecialKey = Key::kNone;
        mModifiers.clear();
    }

    bool Keyboard::is_modifier_pressed(KeyModifier modifier) const {
        using std::begin;
        using std::end;
        return std::find(begin(mModifiers), end(mModifiers), modifier) != end(mModifiers);
    }

    Key Keyboard::get_pressed_key_type() const {
        int from = ' ';
        int to = from + 95;
        return mKeyCode >= from && mKeyCode < to ? Key::kChar : mSpecialKey;
    }

    int Keyboard::get_pressed_char() const {
        if (is_modifier_pressed(KeyModifier::kShift)) {
            return shift_to_upper(mKeyCode);
        } else {
            return mKeyCode;
        }
    }
}
