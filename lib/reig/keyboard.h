#ifndef REIG_KEYBOARD_H
#define REIG_KEYBOARD_H

namespace reig::detail {
    class Keyboard {
    public:
        Keyboard() = default;

        Keyboard(const Keyboard& other) = delete;

        Keyboard(Keyboard&& other) = delete;

        Keyboard& operator=(const Keyboard& other) = delete;

        Keyboard& operator=(Keyboard&& other) = delete;

        void press(int keycode);

        void reset();

        bool is_char_key_pressed() const;

        int get_pressed_char_key() const;

    private:
        int mKeyCode = 0;
    };
}

#endif //REIG_KEYBOARD_H
