#ifndef REIG_KEYBOARD_H
#define REIG_KEYBOARD_H

namespace reig::detail {
    enum class Key {
        UNKNOWN,
        CHAR,
        BACKSPACE
    };

    class Keyboard {
    public:
        Keyboard() = default;

        Keyboard(const Keyboard& other) = delete;

        Keyboard(Keyboard&& other) = delete;

        Keyboard& operator=(const Keyboard& other) = delete;

        Keyboard& operator=(Keyboard&& other) = delete;

        void press(int keycode);

        void reset();

        Key get_pressed_key_type() const;

        int get_pressed_key() const;

    private:
        int mKeyCode = 0;
    };
}

#endif //REIG_KEYBOARD_H
