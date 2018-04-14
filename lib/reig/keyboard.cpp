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

    bool Keyboard::is_char_key_pressed() const {
        int from = ' ';
        int to = from + 95;
        return mKeyCode > from && mKeyCode < to;
    }

    int Keyboard::get_pressed_char_key() const {
        return mKeyCode;
    }
}
