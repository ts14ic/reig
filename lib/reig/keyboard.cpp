#include "keyboard.h"
#include "context.h"
#include <vector>
#include <algorithm>

namespace reig::detail {
    void Keyboard::press(int keycode) {
        mKeyCode = keycode;
    }

    void Keyboard::reset() {
        mKeyCode = 0;
    }

    Key Keyboard::get_pressed_key_type() const {
        int from = ' ';
        int to = from + 95;
        return mKeyCode >= from && mKeyCode < to ? Key::CHAR :
               mKeyCode == '\b' ? Key::BACKSPACE :
               Key::UNKNOWN;
    }

    int Keyboard::get_pressed_key() const {
        return mKeyCode;
    }
}
