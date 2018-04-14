#include "keyboard.h"
#include <vector>
#include <algorithm>

namespace reig::detail {
    void Keyboard::press(int keycode) {
        mKeyCodes.push_back(keycode);
    }

    void Keyboard::release(int keycode) {
        auto it = std::remove_if(begin(mKeyCodes), end(mKeyCodes), [keycode](int containedKeyCode) {
            return containedKeyCode == keycode;
        });
        mKeyCodes.erase(it, end(mKeyCodes));
    }

    void Keyboard::reset() {
        mKeyCodes.clear();
    }
}
