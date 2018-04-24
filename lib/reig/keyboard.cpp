#include "keyboard.h"
#include "context.h"
#include <vector>
#include <algorithm>

namespace reig::detail {
    void Keyboard::press_key(int key) {
        _key_code = key;
        _special_key = Key::kNone;
    }

    void Keyboard::press_special_key(Key key) {
        if (key != Key::kChar) {
            _key_code = 0;
            _special_key = key;
        }
    }

    void Keyboard::press_modifier(KeyModifier modifier) {
        if (!is_modifier_pressed(modifier)) {
            _modifiers.push_back(modifier);
        }
    }

    void Keyboard::reset() {
        _key_code = 0;
        _special_key = Key::kNone;
        _modifiers.clear();
    }

    bool Keyboard::is_modifier_pressed(KeyModifier modifier) const {
        using std::begin;
        using std::end;
        return std::find(begin(_modifiers), end(_modifiers), modifier) != end(_modifiers);
    }

    Key Keyboard::get_pressed_key_type() const {
        int from = ' ';
        int to = from + 95;
        return _key_code >= from && _key_code < to ? Key::kChar : _special_key;
    }

    int Keyboard::get_pressed_char() const {
        if (is_modifier_pressed(KeyModifier::kShift)) {
            return shift_to_upper(_key_code);
        } else {
            return _key_code;
        }
    }
}
